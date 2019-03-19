/*
 * Sampler.h
 *
 *  Created on: 2019-3-19
 *      Author: Leo
 */

#ifndef SAMPLER_H_
#define SAMPLER_H_

#define ADC1_INT_PRIORITY 0  // highest priority

void ADCInit();
void ADC_ISR(void);


#endif /* SAMPLER_H_ */
