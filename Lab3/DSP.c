/*
 * DSP.c
 *
 *  Created on: 2019Äê4ÔÂ22ÈÕ
 *      Author: Leo
 */


#include <stdint.h>
#include <stdbool.h>

#include "driverlib/comp.h"
#include "driverlib/pin_map.h"


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
#include "hwDebug.h"

uint32_t period, last_count, counted_periods, accumulated_period , avgPeriod;


void DSPInit(void )
{
    setupCompartor();
    setupCapture();

}

void setupCompartor()
{
    // part of frequency counter
    SysCtlPeripheralEnable(SYSCTL_PERIPH_COMP0);
    ComparatorRefSet(COMP_BASE, COMP_REF_1_65V);
    ComparatorConfigure(COMP_BASE, 1, COMP_TRIG_NONE | COMP_ASRCP_REF | COMP_OUTPUT_INVERT);

    // configure GPIO for comparator input C1- at BoosterPack Connector #1 pin 3    //c1- -> pc4
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeComparator(GPIO_PORTC_BASE, GPIO_PIN_4);

    // configure GPIO for comparator output C1o at BoosterPack Connector #1 pin 15 //c1o -> pd1 ... pd0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeComparatorOutput(GPIO_PORTD_BASE, GPIO_PIN_1);
    GPIOPinConfigure(GPIO_PD1_C1O);


}


void setupCapture ()
{
    // Timer in Capture Mode

    // configure GPIO PD0 as timer input T0CCP0 at BoosterPack Connector #1 pin 14
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeTimer(GPIO_PORTD_BASE, GPIO_PIN_0);
    GPIOPinConfigure(GPIO_PD0_T0CCP0);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerDisable(TIMER0_BASE, TIMER_BOTH);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME_UP);
    TimerControlEvent(TIMER0_BASE, TIMER_A, TIMER_EVENT_POS_EDGE);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 0xffff);   // use maximum load value
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0xff); // use maximum prescale value
    TimerIntEnable(TIMER0_BASE, TIMER_CAPA_EVENT);
    TimerEnable(TIMER0_BASE, TIMER_A);
}


void captureHwi_ISR(UArg arg) {
    TimerIntClear(TIMER0_BASE, TIMER_CAPA_EVENT); // clear timer 0a capture interrupt flag

    uint32_t count = TimerValueGet(TIMER0_BASE, TIMER_A); //read captured timer count

    period = (count - last_count) & 0xffffff; //deal w/ overflow
    last_count = count;

    static IArg keySettingGate;
    keySettingGate = GateHwi_enter(gateHwi0);

    accumulated_period += period;
    counted_periods++;

    GateHwi_leave(gateHwi1, keySettingGate);

}

void periodClockSwi(UArg arg0) {

    Semaphore_post(freqSem);


    debugPin0 ^= 1;
//    debugPin0= 0;
}

void FrequencyTask(UArg arg1, UArg arg2)
{
    while(1){
        Semaphore_pend(freqSem,BIOS_WAIT_FOREVER);
        static IArg keySettingGate;
        keySettingGate = GateHwi_enter(gateHwi0);

        avgPeriod =    accumulated_period / counted_periods ;
        accumulated_period =0;
        counted_periods=0;

        GateHwi_leave(gateHwi1, keySettingGate);
    }

}

///////////////////////// genearting PWM

#include "driverlib/pwm.h"

#define PWM_PERIOD 258 // PWM period = 2^8 + 2 system clock cycles

void PWMInit()
{
    // use M0PWM1, at GPIO PF1, which is BoosterPack Connector #1 pin 40
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1); // PF1 = M0PWM1
    GPIOPinConfigure(GPIO_PF1_M0PWM1);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
    // configure the PWM0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_1); // use system clock
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, PWM_PERIOD);
    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, PWM_PERIOD/2); // initial 50% duty cycle
    PWMOutputInvert(PWM0_BASE, PWM_OUT_1_BIT, true); // invert PWM output
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true); // enable PWM output
    PWMGenEnable(PWM0_BASE, PWM_GEN_0); // enable PWM generator
    // enable PWM interrupt in the PWM peripheral
    PWMGenIntTrigEnable(PWM0_BASE, PWM_GEN_0, PWM_INT_CNT_ZERO);
    PWMIntEnable(PWM0_BASE, PWM_INT_GEN_0);
}














