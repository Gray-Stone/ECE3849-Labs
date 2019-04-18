/*
 * sampler.c
 *
 *  Created on: 2019-3-19
 *      Author: Leo
 */



#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "sysctl_pll.h"
#include "driverlib/timer.h"

#include "inc/tm4c1294ncpdt.h"

#include "sampler.h"
#include "buttons.h"
#include "hwDebug.h"
#include "globalSetting.h"



extern uint32_t gSystemClock;   // [Hz] system clock frequency
volatile int32_t gADCBufferIndex = ADC_BUFFER_SIZE - 1;  // latest sample index
volatile uint16_t gADCBuffer[ADC_BUFFER_SIZE];           // circular buffer
volatile uint32_t gADCErrors;                       // number of missed ADC deadlines
uint16_t waveformBuffer[SCREENSIZE];  //TODO is waveform processing gonna use this as well?
uint16_t FFTBuffer[FFTBufferSize ]; // the seperate buffer for FFT output.

// DMA global
#include "driverlib/udma.h"
#pragma DATA_ALIGN(gDMAControlTable, 1024) // address alignment required
tDMAControlTable gDMAControlTable[64];     // uDMA control table (global)


// Direct ADC trigger ISR init
void ADCInit()
{
    // Initialize ADC1 and input peripheral for lab 1, step 2:
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP); //AIN3->PE0 is the input pin
    GPIOPinTypeADC(GPIO_PORTP_BASE, GPIO_PIN_0); // GPIO setup for analog input AIN3

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // initialize ADC peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    // ADC clock
    uint32_t pll_frequency = SysCtlFrequencyGet(CRYSTAL_FREQUENCY);
    uint32_t pll_divisor = (pll_frequency - 1) / (16 * ADC_SAMPLING_RATE) + 1; //round up

    ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);
    ADCSequenceDisable(ADC1_BASE, 0);      // choose ADC1 sequence 0; disable before configuring
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_ALWAYS, 0);    // specify the "Always" trigger
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0,  ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);// in the 0th step, sample channel 3 (AIN3)
                                  // enable interrupt, and make it the end of sequence
    ADCSequenceEnable(ADC1_BASE, 0);       // enable the sequence.  it is now sampling
    ADCIntEnable(ADC1_BASE, 0);            // enable sequence 0 interrupt in the ADC1 peripheral

}

// DMA ADC dumping init.



void ADCDMAInit()
{
    // ADC part
    // Initialize ADC1 and input peripheral for lab 1, step 2:
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOP); //AIN3->PE0 is the input pin
    GPIOPinTypeADC(GPIO_PORTP_BASE, GPIO_PIN_0); // GPIO setup for analog input AIN3

    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); // initialize ADC peripherals
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    // ADC clock
    uint32_t pll_frequency = SysCtlFrequencyGet(CRYSTAL_FREQUENCY);
    uint32_t pll_divisor = (pll_frequency - 1) / (16 * ADC_SAMPLING_RATE) + 1; //round up

    ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);
    ADCSequenceDisable(ADC1_BASE, 0);      // choose ADC1 sequence 0; disable before configuring
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_ALWAYS, 0);    // specify the "Always" trigger
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0,  ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);// in the 0th step, sample channel 3 (AIN3)
                                  // enable interrupt, and make it the end of sequence
    ADCSequenceEnable(ADC1_BASE, 0);       // enable the sequence.  it is now sampling
    ADCIntEnable(ADC1_BASE, 0);            // enable sequence 0 interrupt in the ADC1 peripheral

    // DMA part

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    uDMAEnable();
    uDMAControlBaseSet(gDMAControlTable);

    uDMAChannelAssign(UDMA_CH24_ADC1_0); // assign DMA channel 24 to ADC1 sequence 0
    uDMAChannelAttributeDisable(UDMA_SEC_CHANNEL_ADC10, UDMA_ATTR_ALL);

    // primary DMA channel = first half of the ADC buffer
    uDMAChannelControlSet(UDMA_SEC_CHANNEL_ADC10 | UDMA_PRI_SELECT,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_4);
    uDMAChannelTransferSet(UDMA_SEC_CHANNEL_ADC10 | UDMA_PRI_SELECT,
        UDMA_MODE_PINGPONG, (void*)&ADC1_SSFIFO0_R,
        (void*)&gADCBuffer[0], ADC_BUFFER_SIZE/2);

    // alternate DMA channel = second half of the ADC buffer
    uDMAChannelControlSet(UDMA_SEC_CHANNEL_ADC10 | UDMA_ALT_SELECT,
        UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_4);
    uDMAChannelTransferSet(UDMA_SEC_CHANNEL_ADC10 | UDMA_ALT_SELECT,
        UDMA_MODE_PINGPONG, (void*)&ADC1_SSFIFO0_R,
        (void*)&gADCBuffer[ADC_BUFFER_SIZE/2], ADC_BUFFER_SIZE/2);

    uDMAChannelEnable(UDMA_SEC_CHANNEL_ADC10);


    ADCSequenceDMAEnable(ADC1_BASE, 0); // enable DMA for ADC1 sequence 0
    ADCIntEnableEx(...);                // enable ADC1 sequence 0 DMA interrupt

}







// ADC ISR

void ADC_ISR(UArg arg)
{
    //System_printf("Entered ADC_ISR\n");

    ADC1_ISC_R |=1 ; // clear ADC1 sequence0 interrupt flag in the ADCISC register
#ifdef SampleTIMING
    debugPin0= 1;
#endif
    if (ADC1_OSTAT_R & ADC_OSTAT_OV0) { // check for ADC FIFO overflow
        gADCErrors++;                   // count errors
        ADC1_OSTAT_R = ADC_OSTAT_OV0;   // clear overflow condition
    }
    gADCBuffer[
               gADCBufferIndex = ADC_BUFFER_WRAP(gADCBufferIndex + 1)
               ] = ADC1_SSFIFO0_R;               // read sample from the ADC1 sequence 0 FIFO
#ifdef SampleTIMING
    debugPin0= 0;
#endif

}

// DMA ISR

volatile bool gDMAPrimary = true; // is DMA occurring in the primary channel?

void DMA_ISR(void)  // DMA (lab3)
{
    ADCIntClearEx(...); // clear the ADC1 sequence 0 DMA interrupt flag

    // Check the primary DMA channel for end of transfer, and restart if needed.
    if (uDMAChannelModeGet(UDMA_SEC_CHANNEL_ADC10 | UDMA_PRI_SELECT) ==
            UDMA_MODE_STOP) {
        uDMAChannelTransferSet(...); // restart the primary channel (same as setup)
        gDMAPrimary = false;    // DMA is currently occurring in the alternate buffer
    }

    // Check the alternate DMA channel for end of transfer, and restart if needed.
    // Also set the gDMAPrimary global.
    <...>

    // The DMA channel may be disabled if the CPU is paused by the debugger.
    if (!uDMAChannelIsEnabled(UDMA_SEC_CHANNEL_ADC10)) {
        uDMAChannelEnable(UDMA_SEC_CHANNEL_ADC10);  // re-enable the DMA channel
    }
}













bool triggerFound = false ;// determent if a trigger is found, reset to False every loop


// go though the gathered waveform trying to find a trigger case. priority 14
//trigger on triggerFindSem
void triggerFindTask (UArg arg1, UArg arg2)
{
//    ADCInit();
    IntMasterEnable();

    uint32_t i =0;
    int32_t triggerIndex,  triggerIndexInit ,startIndex ;   // sotre the trigger locations.
    uint16_t  sample, sampleFuture; // the two samples to compare to

    uint16_t triggerLevel ;
    char edgetype ;

    while(1)
    {
        // wait for start
        Semaphore_pend(triggerFindSem,BIOS_WAIT_FOREVER);

        if (settings.FFT)   // in the case of FFT mode
        {
            triggerIndex = gADCBufferIndex ;
            for ( i=0;i< FFTBufferSize ; ++i )  // copy enough samples into FFT buffer for process.
                FFTBuffer[i] = gADCBuffer[ ADC_BUFFER_WRAP(triggerIndex + i) ];
            goto endloopTag; // use go to to skip the next non FFT section

        }

        // this section is the case of normal mode
        triggerLevel = settings.triggerLevel;
        edgetype = settings.edge ;


        //initialize the trigger index, and preserve it:
        triggerIndex = ADC_BUFFER_WRAP(gADCBufferIndex - 64);   //half a screen (128/2 = 64) behind gADCBufferIndex (most recent sample index in FIFO)
        triggerIndexInit = triggerIndex;                        //log the initial trigger index, if nothing found the initial index is going to be used
        startIndex = ADC_BUFFER_WRAP( triggerIndexInit - (SCREENSIZE/2) );  // starting point for recording waveform.

        triggerFound = false;

        sample = gADCBuffer[triggerIndex]; //read the initial sample at this index location

        while (triggerIndex != ADC_BUFFER_WRAP( triggerIndexInit - (ADC_BUFFER_SIZE / 2))) // keep looping until trigger is about to hit the newly read data.
        {
            triggerIndex = ADC_BUFFER_WRAP(triggerIndex - 1);
            sampleFuture = sample;
            sample =  gADCBuffer[triggerIndex];
            if (triggerCheck ( sample, sampleFuture, triggerLevel, edgetype ) ) // found a rising edge at trigger
            {
                startIndex = ADC_BUFFER_WRAP( triggerIndex - (SCREENSIZE/2) );
                triggerFound = true ;
            }
        }

//        if (triggerFound) { ; }                     // This is left for normal trigger mode

        for (i = 0; i < SCREENSIZE ; i++)           // careful about 0 index.
        {
            waveformBuffer[i] = gADCBuffer[ADC_BUFFER_WRAP ( startIndex + i) ];
        }

        endloopTag:
        Semaphore_post(processingSem);
    }
}



bool triggerCheck (int16_t sample, int16_t sampleFuture, int16_t triggerLevel, char edgetype) // edgetype 0 for rising, 1 for falling.
{
    if (edgetype == 0 )
    {
        if ( sample < triggerLevel && sampleFuture >= triggerLevel ) // rising edge case
                return true ;
    }
    else
    {
        if ( sample > triggerLevel && sampleFuture <= triggerLevel )    // falling edge case.
               return true ;
    }
   return false;
}










