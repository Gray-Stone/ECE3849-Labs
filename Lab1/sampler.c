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

#include "sampler.h"

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
    ADCClockConfigSet(ADC0_BASE, ADC_CLOCK_SRC_PLL | ADC_CLOCK_RATE_FULL, pll_divisor);
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






