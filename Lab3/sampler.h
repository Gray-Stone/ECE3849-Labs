/*
 * Sampler.h
 *
 *  Created on: 2019-3-19
 *      Author: Leo
 */

#ifndef SAMPLER_H_
#define SAMPLER_H_

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/cfg/global.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>



#define ADC1_INT_PRIORITY 0  // highest priority

#define ADC_BUFFER_SIZE 2048                             // size must be a power of 2
#define ADC_BUFFER_WRAP(i) ((i) & (ADC_BUFFER_SIZE - 1)) // index wrapping macro
#define ADC_OFFSET 2043
#define VIN_RANGE 3.3           //total input range of the ADC, in volts
#define ADC_BITS 12             //number of bits in the ADC
#define SCREENSIZE 128          // number of pixs across screen

#define FFTBufferSize 1024



extern volatile int32_t gADCBufferIndex ;  // latest sample index
extern volatile uint16_t gADCBuffer[ADC_BUFFER_SIZE];           // circular buffer
extern volatile uint32_t gADCErrors;                       // number of missed ADC deadlines

extern volatile uint16_t sampleTemp;
extern volatile uint16_t sampleTemp2;
extern uint16_t waveformBuffer[SCREENSIZE];
extern uint16_t FFTBuffer[FFTBufferSize ];


void ADCInit();
void ADC_ISR(UArg arg);
void alwaysTriggerADC(void);
void timerTriggerADC(uint32_t denominator);

bool triggerCheck (int16_t sample, int16_t sampleFuture, int16_t triggerLevel, char edgetype);



#endif /* SAMPLER_H_ */
