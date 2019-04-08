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

uint16_t samples2Draw[SCREENSIZE];  //TODO is waveform processing gonna use this as well?


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

    //////**************************** TESTING CODE
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);    //timer 3 used for cpu stuff, let's use timer 4
//
//    TimerDisable(TIMER4_BASE, TIMER_BOTH);
//    TimerConfigure(TIMER4_BASE, TIMER_CFG_PERIODIC); //want TIMER_CFG_PERIODIC ?
//    TimerLoadSet(TIMER4_BASE, TIMER_A, (gSystemClock/400000) - 1); // 10 ms interval (timeScale/20)
//
//    TimerControlTrigger(TIMER4_BASE, TIMER_A, true);

    //////****************************
    ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);
    ADCSequenceDisable(ADC1_BASE, 0);      // choose ADC1 sequence 0; disable before configuring
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_ALWAYS, 0);    // specify the "Always" trigger
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0,  ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);// in the 0th step, sample channel 3 (AIN3)
                                  // enable interrupt, and make it the end of sequence
    ADCSequenceEnable(ADC1_BASE, 0);       // enable the sequence.  it is now sampling
    ADCIntEnable(ADC1_BASE, 0);            // enable sequence 0 interrupt in the ADC1 peripheral
    //IntPrioritySet(INT_ADC1SS0, ADC1_INT_PRIORITY);          // set ADC1 sequence 0 interrupt priority
    //IntEnable(INT_ADC1SS0);               // enable ADC1 sequence 0 interrupt in int. controller
    //end lab 1 step 2
}

// for 20us mode.
void alwaysTriggerADC(void) {
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_ALWAYS, 0);
}

// for all other samping mode.
void timerTriggerADC(uint32_t denominator){
    TimerDisable(TIMER4_BASE, TIMER_BOTH);
    TimerLoadSet(TIMER4_BASE, TIMER_A, (gSystemClock/denominator) - 1);
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_TIMER, 0);
    TimerEnable(TIMER4_BASE, TIMER_BOTH);
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


// go though the gathered waveform trying to find a trigger case. priority 14
//trigger on triggerFindSem
void triggerFindTask (UArg arg1, UArg arg2)
{
    uint32_t i =0;
    int32_t triggerIndex,  triggerIndexInit ,startIndex ;   // sotre the trigger locations.
    uint16_t  sample, sampleFuture; // the two samples to compare to
    bool triggerFound = false ;// determent if a trigger is found, reset to False every loop

    uint16_t triggerLevel ;
    char edgetype ;

    while(1)
    {
        // wait for start
        Semaphore_pend(triggerFindSem,BIOS_WAIT_FOREVER);

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
            samples2Draw[i] = gADCBuffer[ADC_BUFFER_WRAP ( startIndex + i) ];
        }
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










