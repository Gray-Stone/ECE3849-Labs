/*
 * Sampler.h
 *
 *  Created on: 2019-3-19
 *      Author: Leo
 */

#ifndef SAMPLER_H_
#define SAMPLER_H_

#define ADC1_INT_PRIORITY 0  // highest priority

#define ADC_BUFFER_SIZE 2048                             // size must be a power of 2
#define ADC_BUFFER_WRAP(i) ((i) & (ADC_BUFFER_SIZE - 1)) // index wrapping macro

#define ADC_OFFSET 2048

#define VIN_RANGE 3.3 //total input range of the ADC, in volts

#define ADC_BITS 12 //number of bits in the ADC

extern volatile int32_t gADCBufferIndex ;  // latest sample index
extern volatile uint16_t gADCBuffer[ADC_BUFFER_SIZE];           // circular buffer
extern volatile uint32_t gADCErrors;                       // number of missed ADC deadlines

extern volatile uint16_t sampleTemp;
extern volatile uint16_t sampleTemp2;



void ADCInit();
void ADC_ISR(void);


#endif /* SAMPLER_H_ */
