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

uint32_t gSystemClock = 120000000; // [Hz] system clock frequency

/*
 *  ======== main ========
 */
int main(void)


{
    IntMasterDisable();
    //System_printf("Entered Main()\n");
    ADCInit();
    screenInit();
    ButtonInit();


    // hardware initialization goes in front of the tasks
    debugPinsInit();
    /* Start BIOS */
    BIOS_start();

    return (0);
}

void task0_func(UArg arg1, UArg arg2)
{
    IntMasterEnable();

    while (true) {
        //System_printf("Entered task0\n");
        // do nothing
    }
}


/*

#include <stdint.h>
#include <stdbool.h>
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "inc/tm4c1294ncpdt.h"
#include "driverlib/timer.h"
#include "inc/hw_memmap.h"


#include "Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include "buttons.h"
#include "sampler.h"
#include "hwDebug.h"
#include "screenControl.h"
#include "btnFIFO.h"

#define SCREENSIZE 128

//timing debug measuring option
#define TriggerTIMING

//global variables
uint32_t gSystemClock; // [Hz] system clock frequency

//functions
bool triggerCheck (int16_t sample, int16_t sampleFuture, int16_t triggerLevel, char edgetype);
int32_t findTrigger(int16_t triggerLevel , char edgetype);
uint16_t changeVoltPerDiv(char direction, uint16_t oldVoltPerDiv);
uint32_t measure_ISR_CPU(void);
uint32_t changeTimePerDiv(char direction, uint16_t oldTimePerDiv );


int main(void)
{
    IntMasterDisable();

    // Enable the Floating Point Unit, and permit ISRs to use it
    FPUEnable();
    FPULazyStackingEnable();

    // Initialize the system clock to 120 MHz
    gSystemClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, 120000000);

    // initialize all sub components.
    screenInit();
    ButtonInit();
    ADCInit();
    debugPinsInit();

    // initialize timer 3 in one-shot mode for polled timing
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    TimerDisable(TIMER3_BASE, TIMER_BOTH);
    TimerConfigure(TIMER3_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER3_BASE, TIMER_A, (gSystemClock/100) - 1); //

    uint32_t count_unloaded = measure_ISR_CPU();
    uint32_t count_loaded = 0;
    float cpu_load = 0;

    IntMasterEnable();

    char edgetype = 0; // the variable for setting the trigger edge type: 0 for rising.
    uint16_t mVPerDiv  = 100; // set the voltage scale.
    uint32_t usPerDiv = 20; // set the time scale.
    uint16_t   triggerLevel = ADC_OFFSET;
    int32_t     triggerIndex = 0 , startIndex = 0;
    uint16_t samples2Draw[SCREENSIZE];
    int32_t i; //for general looping needs


    while (true) {

        // measure CPU load.
        count_loaded = measure_ISR_CPU();
        cpu_load = 1.0f - (float)count_loaded/count_unloaded; // compute CPU load

        //find the trigger point.
        triggerIndex = findTrigger(triggerLevel,edgetype);

        //copy samples from 1/2 screen behind to 1/2 ahead the trigger index into local buffer
        startIndex = ADC_BUFFER_WRAP( triggerIndex - (SCREENSIZE/2) );
        for (i = 0; i < SCREENSIZE ; i++)  // careful about 0 index.
        {
            samples2Draw[i] = gADCBuffer[ADC_BUFFER_WRAP ( startIndex + i) ];
        }

        // draw this onto the screen
        drawScreen(samples2Draw, SCREENSIZE , mVPerDiv, edgetype, cpu_load);

        // make sure this button thing is the last section in code.
        uint32_t btnData = fifoPoll();
        if ( btnData == 0 )
            continue; // short circuit skip all the rest of the button checks.
        // Booster Pack btn S1 change trigger.  0x0004
        // Booster Pack btn S2 change run/stop. 0x0008
        // Booster Pack joy up increase V/div   0x0080
        // Booster Pack joy down decrease V/div 0x0100
        // Booster Pack joy Left more us/div    0x0040
        // Booster Pack joy Right less us/div   0x0020
        if ( btnData & 0x0004 ) // case of btn S1 is pushed. flip triggerType
            edgetype^=0x01 ;
        if ( btnData & 0x0008 ) // case of btnS2 is pushed.
            ; // currently we don't care about this.
        if ( btnData & 0x0080 ) // case of increase voltage scale.
            mVPerDiv = changeVoltPerDiv(1, mVPerDiv);
        if ( btnData & 0x0100 ) // case of decrease voltage scale
            mVPerDiv = changeVoltPerDiv(0, mVPerDiv);
        if (btnData & 0x0040)   // case of increase time scale
            usPerDiv= changeTimePerDiv(1,usPerDiv);
        if (btnData & 0x0020)   // case of decrease time scale
            usPerDiv= changeTimePerDiv(0,usPerDiv);
    }
}




// go though the gathered waveform trying to find a trigger case.
int32_t findTrigger(int16_t triggerLevel , char edgetype)
{
#ifdef TriggerTIMING
    debugPin1= 1;
#endif
    int32_t triggerIndex,  triggerIndexInit ;   // sotre the trigger locations.
    uint16_t  sample, sampleFuture; // the two samples to compare to

    //initialize the trigger index, and preserve it:
    triggerIndex = ADC_BUFFER_WRAP(gADCBufferIndex - 64);//half a screen (128/2 = 64) behind gADCBufferIndex (most recent sample index in FIFO)
    triggerIndexInit = triggerIndex;//log the initial trigger index, if nothing found the initial index is going to be used
    sample = gADCBuffer[triggerIndex]; //read the initial sample at this index location

    while (triggerIndex != ADC_BUFFER_WRAP( triggerIndexInit - (ADC_BUFFER_SIZE / 2))) // keep looping until trigger is about to hit the newly read data.
    {
        triggerIndex = ADC_BUFFER_WRAP(triggerIndex - 1);
        sampleFuture = sample;
        sample =  gADCBuffer[triggerIndex];
        if (triggerCheck ( sample, sampleFuture, triggerLevel, edgetype ) ) // found a rising edge at trigger
        {
#ifdef TriggerTIMING
    debugPin1= 0;
#endif
            return  triggerIndex ;
        }
    }
    // case of nothing found
#ifdef TriggerTIMING
    debugPin1= 0;
#endif
    // just plot whatever is newest.
    return triggerIndexInit;
}



bool triggerCheck (int16_t sample, int16_t sampleFuture, int16_t triggerLevel, char edgetype) // edgetype 0 for rising, 1 for falling.
{
    if (edgetype == 0 )
    {
        if ( sample < triggerLevel && sampleFuture >= triggerLevel ) // rising edge case
                return true ;
    }
    else
    {
        if ( sample > triggerLevel && sampleFuture <= triggerLevel )    // falling edge case.
               return true ;
    }
   return false;
}


// return the voltage scale according to the given direction
// direction is 0 for down, anything else for up.
//
uint16_t changeVoltPerDiv(char direction, uint16_t oldVoltPerDiv )
{
    uint16_t newVoltPerDiv = oldVoltPerDiv;

    switch (oldVoltPerDiv)
    {
    case 100: newVoltPerDiv = direction ? 200 : 100; break;
    case 200: newVoltPerDiv = direction ? 500 : 100; break;
    case 500: newVoltPerDiv = direction ? 1000 : 200; break;
    case 1000: newVoltPerDiv = direction ? 1000 : 500;
    }
    return newVoltPerDiv;
}

//returns the new time per division (in uS), according to the direction given
//direction is 0 for zoom in, anything else for zoom out
uint32_t changeTimePerDiv(char direction, uint16_t oldTimePerDiv )
{
    uint16_t newTimePerDiv = oldTimePerDiv;

    switch (oldTimePerDiv)
    {
    case 20:
        if (direction) {
            newTimePerDiv = 50;
            timerTriggerADC(400000);
        }
        break;
    case 50:
        if (direction) {
            newTimePerDiv = 100;
            timerTriggerADC(10000);
        }
        else {
            newTimePerDiv = 20;
            alwaysTriggerADC();
        }
        break;
    case 100:
        if (!direction) {
            newTimePerDiv = 50;
            timerTriggerADC(400000);
        }
        break;
    }
    return newTimePerDiv;
}

uint32_t measure_ISR_CPU(void)
{
    uint32_t i = 0;
    TimerIntClear(TIMER3_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER3_BASE, TIMER_A); // start one-shot timer
    while (!(TimerIntStatus(TIMER3_BASE, false) & TIMER_TIMA_TIMEOUT))
        i++;
    return i;
}

 */
