/*
 * hwDebug.h
 *
 *  Created on: 2019Äê3ÔÂ21ÈÕ
 *      Author: Leo
 */

#ifndef HWDEBUG_H_
#define HWDEBUG_H_


void pulsePC7Init();

#define DEBUG_SET      HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 1
#define DEBUG_CLEAR     HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 0


#endif /* HWDEBUG_H_ */
