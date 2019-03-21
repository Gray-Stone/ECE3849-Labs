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
#include "Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include "buttons.h"
#include "sampler.h"
#include "hwDebug.h"

uint32_t gSystemClock; // [Hz] system clock frequency
volatile uint32_t gTime = 8345; // time in hundredths of a second
char edgetype; // the variable for seeting the trigger edge type

int32_t findTrigger(int16_t triggerLevel , char edgetype);


int main(void)
{
    IntMasterDisable();

    // Enable the Floating Point Unit, and permit ISRs to use it
    FPUEnable();
    FPULazyStackingEnable();

    // Initialize the system clock to 120 MHz
    gSystemClock = SysCtlClockFreqSet(SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_480, 120000000);

    Crystalfontz128x128_Init(); // Initialize the LCD display driver
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP); // set screen orientation

    tContext sContext;
    GrContextInit(&sContext, &g_sCrystalfontz128x128); // Initialize the grlib graphics context
    GrContextFontSet(&sContext, &g_sFontFixed6x8); // select font

    char str1[50];   // string buffer line 1
    char str2[50];  //string buffer line 2
    // full-screen rectangle
    tRectangle rectFullScreen = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1};

    ButtonInit();
    ADCInit();
    IntMasterEnable();

    char edgetype; // the variable for seeting the trigger edge type
    int16_t   triggerLevel = 200 ;
    int36_t temp =0;
    bool triggerFound =0;


    while (true) {

        temp = findTrigger(triggerLevel,edgetype);
        //
        //


        pulsePP2Init();

    }
}





int32_t findTrigger(int16_t triggerLevel , char edgetype)
{
    int32_t triggerIndex,  triggerIndexOut ;
    int16_t  sample, sampleFuture;
    //trigger search:
    //initialize the trigger index, and preserve it:
    triggerIndex = ADC_BUFFER_WRAP(gADCBufferIndex - 64);//half a screen (128/2 = 64) behind gADCBufferIndex (most recent sample index in FIFO)
    triggerIndexOut = triggerIndex;//log the initial trigger index, if nothing found the initial index is going to be used
    sample = gADCBuffer[triggerIndex]; //read the initial sample at this index location

    // rising edge trigger.
    
    for ( ; triggerIndex > ADC_BUFFER_WRAP( triggerIndex - ADC_BUFFER_SIZE / 2) ;)   { //stop when sample < offset && future > offset
        triggerIndex = ADC_BUFFER_WRAP(triggerIndex--);
        sampleFuture = sample;
        sample =  gADCBuffer[triggerIndex];


        if (triggerCheck ( sample, sampleFuture, triggerLevel, edgetype ) ) // found a rising edge at trigger
        {
            return  triggerIndex ; 
        }

    }
    return triggerIndexOut;
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
