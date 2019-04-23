/*
 * DSP.h
 *
 *  Created on: 2019Äê4ÔÂ22ÈÕ
 *      Author: Leo
 */

#ifndef DSP_H_
#define DSP_H_

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/cfg/global.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

void DSPInit(void );
void setupCompartor();
void setupCapture();
void PWMInit();

extern uint32_t period, last_count, counted_periods, accumulated_period;

#endif /* DSP_H_ */
