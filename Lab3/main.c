/*
 * ECE 3849 Lab2 starter project
 *
 * Gene Bogdanov    9/13/2017
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/cfg/global.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"

#include "driverlib/timer.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"

#include "sampler.h"
#include "buttons.h"
#include "screenControl.h"
#include "hwDebug.h"

#include "inc/tm4c1294ncpdt.h"
#include "driverlib/gpio.h"



uint32_t gSystemClock = 120000000; // [Hz] system clock frequency
uint32_t count_unloaded;

/*
 *  ======== main ========
 */
int main(void)


{
    IntMasterDisable(); // int is re emabled at the beginning of the highest priority task.
    // init for all sub components.
    //ADCInit();
    ADCDMAInit();
    screenInit();
    ButtonInit();
    // hardware initialization goes in front of the tasks
    debugPinsInit();



    //setup for CPU measurement
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    TimerDisable(TIMER3_BASE, TIMER_BOTH);
    TimerConfigure(TIMER3_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER3_BASE, TIMER_A, (120000000/100) - 1);

    count_unloaded = measure_ISR_CPU();

    /* Start BIOS */
    BIOS_start();

    return (0);
}

// dummy class that should do nothing.
void task0_func(UArg arg1, UArg arg2)
{
    IntMasterEnable();

    while (true) {
        //System_printf("Entered task0\n");
        // do nothing
    }
}


