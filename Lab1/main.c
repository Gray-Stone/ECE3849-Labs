/**
 * main.c
 *
 * ECE 3849 Lab 0 Starter Project
 * Gene Bogdanov    10/18/2017
 *
 * This version is using the new hardware for B2017: the EK-TM4C1294XL LaunchPad with BOOSTXL-EDUMKII BoosterPack.
 *
 */

//timing measuring option
#define TriggerTIMING
//#define SampleTIMING

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

#define SCREENSIZE 128


//global variables
uint32_t gSystemClock; // [Hz] system clock frequency
volatile uint32_t gTime = 8345; // time in hundredths of a second
char edgetype; // the variable for seeting the trigger edge type


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


    char edgetype = 0; // the variable for setting the trigger edge type: 0 for rising.
    uint16_t   triggerLevel = 200 ;
    int32_t     triggerIndex =0 , startIndex = 0;
    uint16_t samples2Draw[SCREENSIZE];
    int32_t i; //for general looping needs



    while (true) {

        //find the trigger point.
        triggerIndex = findTrigger(triggerLevel,edgetype);

        //copy samples from 1/2 screen behind to 1/2 ahead the trigger index into local buffer
        startIndex = ADC_BUFFER_WRAP( triggerIndex - SCREENSIZE/2 );
        for (i = 0; i < SCREENSIZE ; i++)  // careful about 0 index.
        {
            samples2Draw[i] = gADCBuffer[ADC_BUFFER_WRAP ( startIndex + i) ];
        }

        // draw this onto the screen
        drawSamples(samples2Draw, SCREENSIZE , 200 );
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
    for ( ; triggerIndex == ADC_BUFFER_WRAP( triggerIndexInit - ADC_BUFFER_SIZE / 2) ;)   { //stop when sample < offset && future > offset
        triggerIndex = ADC_BUFFER_WRAP(triggerIndex--);
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
    switch (edgetype)
    {
        case 0:     // case of rising edge
            if ( sample < triggerLevel && sampleFuture >= triggerLevel )
                return true ;
            break;
        case 1:     // falling edge
            if ( sample > triggerLevel && sampleFuture <= triggerLevel )
               return true ;
           break;
    }
   return false;
}

