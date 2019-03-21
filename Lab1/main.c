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


    volatile int32_t triggerIndex,  triggerIndexPreserved;
    int32_t samplesVisited, sample, sampleFuture;


    while (true) {

        //trigger search:
        //initialize the trigger index, and preserve it:
        triggerIndex = ADC_BUFFER_WRAP(gADCBufferIndex - 64);//half a screen (128/2 = 64) behind gADCBufferIndex (most recent sample index in FIFO)
        triggerIndexPreserved = triggerIndex;//preserve the trigger index
        //read the initial sample at this index location
        samplesVisited = 1; //keep track of samples visited, to abort early if needed
        sampleFuture = gADCBuffer[triggerIndex];
        sample = gADCBuffer[triggerIndex];
        while (sample >= ADC_OFFSET || sampleFuture >= ADC_OFFSET){ //stop when sample < offset && future > offset
            triggerIndex = ADC_BUFFER_WRAP(triggerIndex--);
            sampleFuture = sample;
            sample =  gADCBuffer[triggerIndex];
            samplesVisited++;
            if (samplesVisited >= ADC_BUFFER_SIZE / 2)
                triggerIndex = triggerIndexPreserved; //reset the index
                break; //abort search
        }
        //
        //


        pulsePC7Init();

    }
}
