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



void pulsePC7Init()
{
    // Pin PC7 will be used to pulse output for external debug.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE, GPIO_PIN_7);
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_7,1);
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_7,0);
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_7,1);
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_7,0);
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_7,0);
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_7,0);
    GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_7,1);


//
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 1;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 1;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 1;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 0;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 0;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 0;
//    HWREG(GPIO_PORTC_BASE + (GPIO_O_DATA + (GPIO_PIN_7 << 2))) = 1;


}



