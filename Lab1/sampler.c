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

#define SampleTIMING

extern uint32_t gSystemClock;   // [Hz] system clock frequency


volatile int32_t gADCBufferIndex = ADC_BUFFER_SIZE - 1;  // latest sample index
volatile uint16_t gADCBuffer[ADC_BUFFER_SIZE];           // circular buffer
volatile uint32_t gADCErrors;                       // number of missed ADC deadlines

volatile uint16_t sampleTemp;
volatile uint16_t sampleTemp2;

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
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);    //timer 3 used for cpu stuff, let's use timer 4

    TimerDisable(TIMER4_BASE, TIMER_BOTH);
    TimerConfigure(TIMER4_BASE, TIMER_CFG_PERIODIC); //want TIMER_CFG_PERIODIC ?
    TimerLoadSet(TIMER4_BASE, TIMER_A, (gSystemClock/400000) - 1); // 10 ms interval (timeScale/20)

    TimerControlTrigger(TIMER4_BASE, TIMER_A, true); //we use timer A for cpu measurement, so use B for this

    //ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_TIMER, 0);

    //TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    //TimerEnable(TIMER4_BASE, TIMER_BOTH);
    //////*****************************


    ADCClockConfigSet(ADC1_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);
    ADCSequenceDisable(ADC1_BASE, 0);      // choose ADC1 sequence 0; disable before configuring
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_ALWAYS, 0);    // specify the "Always" trigger
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0,  ADC_CTL_CH3 | ADC_CTL_IE | ADC_CTL_END);// in the 0th step, sample channel 3 (AIN3)
                                  // enable interrupt, and make it the end of sequence
    ADCSequenceEnable(ADC1_BASE, 0);       // enable the sequence.  it is now sampling
    ADCIntEnable(ADC1_BASE, 0);            // enable sequence 0 interrupt in the ADC1 peripheral
    IntPrioritySet(INT_ADC1SS0, ADC1_INT_PRIORITY);          // set ADC1 sequence 0 interrupt priority
    IntEnable(INT_ADC1SS0);               // enable ADC1 sequence 0 interrupt in int. controller
    //end lab 1 step 2

}

void alwaysTriggerADC(void) {
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_ALWAYS, 0);
}

void timerTriggerADC(uint32_t denominator){
    TimerDisable(TIMER4_BASE, TIMER_BOTH);
    TimerLoadSet(TIMER4_BASE, TIMER_A, (gSystemClock/denominator) - 1);
    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_TIMER, 0);
    TimerEnable(TIMER4_BASE, TIMER_BOTH);
}

/*
 *
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);    //timer 3 used for cpu stuff, let's use timer 4

    TimerDisable(TIMER4_BASE, TIMER_A);
    TimerConfigure(TIMER4_BASE, TIMER_CFG_PERIODIC); //want TIMER_CFG_PERIODIC ?
    TimerLoadSet(TIMER4_BASE, TIMER_A, (gSystemClock/100) - 1); // 10 ms interval
    TimerControlTrigger(TIMER4_BASE, TIMER_A, true); //we use timer A for cpu measurement, so use B for this

    ADCSequenceConfigure(ADC1_BASE, 0, ADC_TRIGGER_TIMER, 0);

    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_BOTH);
 */




// ADC ISR

void ADC_ISR(void)
{

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



/*
// Step 4: ADC sample scaling

int y = LCD_VERTICAL_MAX/2 - (int)roundf(fScale * ((int)sample - ADC_OFFSET));

float fScale = (VIN_RANGE * PIXELS_PER_DIV)/((1 << ADC_BITS) * fVoltsPerDiv);


// Step 5: Button command processing

const char * const gVoltageScaleStr[] = {
    "100 mV", "200 mV", "500 mV", "  1 V"
};


 */




