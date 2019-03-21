/*
 * hwDebug.c
 *
 *  Created on: 2019Äê3ÔÂ21ÈÕ
 *      Author: Leo
 */

#include <stdint.h>
#include <stdbool.h>
#include "hwDebug.h"
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"



void pulsePP2Init()
{
    // Pin PC7 will be used to pulse output for external debug.
//    SysCtlPeripheralEnable(SYSCTL_PERIPP_GPIOC);
    GPIOPinTypeGPIOOutput(GPIO_PORTP_BASE, GPIO_PIN_2);

//    GPIO_PORTC_DATA


//
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 1;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 1;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 1;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 0;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 0;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 0;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 1;


}



