/*
 * DSP.c
 *
 *  Created on: 2019Äê4ÔÂ22ÈÕ
 *      Author: Leo
 */



#include "driverlib/comp.h"
#include "driverlib/pin_map.h"

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

#include "DSP.h"




void DSPInit(void )
{
    setupCompartor();
    setupCapture();

}

void setupCompartor()
{
    // part of frequency counter
    SysCtlPeripheralEnable(SYSCTL_PERIPH_COMP0);
    ComparatorRefSet(COMP_BASE, ...);
    ComparatorConfigure(COMP_BASE, 1, ...);

    // configure GPIO for comparator input C1- at BoosterPack Connector #1 pin 3
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIO?);
    GPIOPinTypeComparator(...);

    // configure GPIO for comparator output C1o at BoosterPack Connector #1 pin 15
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIO?);
    GPIOPinTypeComparatorOutput(...);
    GPIOPinConfigure(...);

}


void setupCapture ()
{
    // Timer in Capture Mode

    // configure GPIO PD0 as timer input T0CCP0 at BoosterPack Connector #1 pin 14
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_0);
    GPIOPinConfigure(...);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerDisable(TIMER0_BASE, TIMER_BOTH);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP);
    TimerControlEvent(TIMER0_BASE, TIMER_A, ...);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 0xffff);   // use maximum load value
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0xff); // use maximum prescale value
    TimerIntEnable(TIMER0_BASE, ...);
    TimerEnable(TIMER0_BASE, TIMER_A);
}
