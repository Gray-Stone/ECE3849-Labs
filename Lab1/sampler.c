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
    ADCSequenceStepConfigure(ADC1_BASE, 0, 0,  );// in the 0th step, sample channel 3 (AIN3)
                                  // enable interrupt, and make it the end of sequence
    ADCSequenceEnable(...);       // enable the sequence.  it is now sampling
    ADCIntEnable(...);            // enable sequence 0 interrupt in the ADC1 peripheral
    IntPrioritySet(...);          // set ADC1 sequence 0 interrupt priority
    IntEnable(...);               // enable ADC1 sequence 0 interrupt in int. controller
    //end lab 1 step 2

}



// ADC ISR

#define ADC_BUFFER_SIZE 2048                             // size must be a power of 2
#define ADC_BUFFER_WRAP(i) ((i) & (ADC_BUFFER_SIZE - 1)) // index wrapping macro
volatile int32_t gADCBufferIndex = ADC_BUFFER_SIZE - 1;  // latest sample index
volatile uint16_t gADCBuffer[ADC_BUFFER_SIZE];           // circular buffer
volatile uint32_t gADCErrors;                       // number of missed ADC deadlines

void ADC_ISR(void)
{
    <...>; // clear ADC1 sequence0 interrupt flag in the ADCISC register
    if (ADC1_OSTAT_R & ADC_OSTAT_OV0) { // check for ADC FIFO overflow
        gADCErrors++;                   // count errors
        ADC1_OSTAT_R = ADC_OSTAT_OV0;   // clear overflow condition
    }
    gADCBuffer[
               gADCBufferIndex = ADC_BUFFER_WRAP(gADCBufferIndex + 1)
               ] = <...>;               // read sample from the ADC1 sequence 0 FIFO
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




