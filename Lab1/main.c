/**
 * main.c
 *
 * ECE 3849 Lab 0 Starter Project
 * Gene Bogdanov    10/18/2017
 *
 * This version is using the new hardware for B2017: the EK-TM4C1294XL LaunchPad with BOOSTXL-EDUMKII BoosterPack.
 *
 */


#include <stdint.h>
#include <stdbool.h>
#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "inc/tm4c1294ncpdt.h"


#include "Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include "buttons.h"
#include "sampler.h"
#include "hwDebug.h"
#include "screenControl.h"
#include "btnFIFO.h"

#define SCREENSIZE 128

//timing measuring option
#define TriggerTIMING
#define SampleTIMING


//global variables
uint32_t gSystemClock; // [Hz] system clock frequency



//functions
bool triggerCheck (int16_t sample, int16_t sampleFuture, int16_t triggerLevel, char edgetype);
int32_t findTrigger(int16_t triggerLevel , char edgetype);


int main(void)
{
    IntMasterDisable();

    // Enable the Floating Point Unit, and permit ISRs to use it
    FPUEnable();
    FPULazyStackingEnable();

    // Initialize the system clock to 120 MHz
    gSystemClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, 120000000);

    screenInit();

    ButtonInit();
    ADCInit();
    debugPinsInit();
    IntMasterEnable();

    // trigger edge controlled by Button S1
    char edgetype = 0; // the variable for setting the trigger edge type: 0 for rising.
    uint16_t mVPerDiv  = 100; // set the voltage scale.
    uint16_t   triggerLevel = ADC_OFFSET;
    int32_t     triggerIndex = 0 , startIndex = 0;
    uint16_t samples2Draw[SCREENSIZE];
    int32_t i; //for general looping needs



    while (true) {

        //find the trigger point.
        triggerIndex = findTrigger(triggerLevel,edgetype);

        //copy samples from 1/2 screen behind to 1/2 ahead the trigger index into local buffer
        startIndex = ADC_BUFFER_WRAP( triggerIndex - (SCREENSIZE/2) );
        for (i = 0; i < SCREENSIZE ; i++)  // careful about 0 index.
        {
            samples2Draw[i] = gADCBuffer[ADC_BUFFER_WRAP ( startIndex + i) ];
        }

        // draw this onto the screen
        drawScreen(samples2Draw, SCREENSIZE , mVPerDiv );

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
            ;
        if ( btnData & 0x0100 ) // case of decrease voltage scale
            ;

//        if (btnData & 0x01)

    }
}





int32_t findTrigger(int16_t triggerLevel , char edgetype)
{
#ifdef TriggerTIMING
    debugPin1= 1;
#endif
    int32_t triggerIndex,  triggerIndexInit ;
    uint16_t  sample, sampleFuture;
    //trigger search:
    //initialize the trigger index, and preserve it:
    triggerIndex = ADC_BUFFER_WRAP(gADCBufferIndex - 64);//half a screen (128/2 = 64) behind gADCBufferIndex (most recent sample index in FIFO)
    triggerIndexInit = triggerIndex;//log the initial trigger index, if nothing found the initial index is going to be used
    sample = gADCBuffer[triggerIndex]; //read the initial sample at this index location

    // rising edge trigger.
    //for ( ; triggerIndex == ADC_BUFFER_WRAP( triggerIndexInit - (ADC_BUFFER_SIZE / 2)) ;)   { //stop when sample < offset && future > offset
    while (triggerIndex != ADC_BUFFER_WRAP( triggerIndexInit - (ADC_BUFFER_SIZE / 2))) {
        //triggerIndex = triggerIndex--;
        triggerIndex = ADC_BUFFER_WRAP(triggerIndex - 1);
        sampleFuture = sample;
        sample =  gADCBuffer[triggerIndex];


        if (triggerCheck ( sample, sampleFuture, triggerLevel, edgetype ) ) // found a rising edge at trigger
        {
            sample +=1 ;
#ifdef TriggerTIMING
    debugPin1= 0;
#endif
            return  triggerIndex ;
        }

    }

#ifdef TriggerTIMING
    debugPin1= 0;
#endif

    return triggerIndexInit;
}



bool triggerCheck (int16_t sample, int16_t sampleFuture, int16_t triggerLevel, char edgetype) // edgetype 0 for rising, 1 for falling.
{
    if (edgetype)
    {
        if ( sample < triggerLevel && sampleFuture >= triggerLevel )
                return true ;
    }
    else
    {
        if ( sample > triggerLevel && sampleFuture <= triggerLevel )
               return true ;
    }
   return false;
}



uint16_t changeVoltPerDiv(char direction, uint16_t oldVoltPerDiv )
{
    if ( oldVoltPerDiv) 
    ;
}