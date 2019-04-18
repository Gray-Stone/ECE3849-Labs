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

#include "sampler.h"
#include "buttons.h"
#include "screenControl.h"

uint32_t gSystemClock = 120000000; // [Hz] system clock frequency

/*
 *  ======== main ========
 */
int main(void)


{
    IntMasterDisable(); // int is re emabled at the beginning of the highest priority task.
    // init for all sub components.
    ADCInit();
    screenInit();
    ButtonInit();
    // hardware initialization goes in front of the tasks
    debugPinsInit();

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


