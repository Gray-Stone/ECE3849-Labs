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

    uint32_t time, mins, secs, discsecs, centiseconds;  // local copy of gTime
    char str1[50];   // string buffer line 1
    char str2[50];  //string buffer line 2
    // full-screen rectangle
    tRectangle rectFullScreen = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1};

    ButtonInit();
    ADCInit();
    IntMasterEnable();

    while (true) {
//        GrContextForegroundSet(&sContext, ClrBlack);
//        GrRectFill(&sContext, &rectFullScreen); // fill screen with black
//        time = gTime; // read shared global only once
        centiseconds = time % 100; //45
        secs = ((time - centiseconds)/100); //8300 -> 83
        discsecs = secs % 60; //23
        mins =(secs / 60);

//        snprintf(str1, sizeof(str1), "Time = %02u:%02u:%02u\0", mins, discsecs, centiseconds); //display the time
//        snprintf(str2, sizeof(str2), "%1u%1u%1u%1u%1u%1u%1u%1u%1u\0", //display the 9 LSb of the button states
//                 (GPIO_STATE>>8)&1, (GPIO_STATE>>7)&1,
//                 (GPIO_STATE>>6)&1, (GPIO_STATE>>5)&1,
//                 (GPIO_STATE>>4)&1, (GPIO_STATE>>3)&1,
//                 (GPIO_STATE>>2)&1, (GPIO_STATE>>1)&1,
//                  GPIO_STATE&1); // convert time to string
//        GrContextForegroundSet(&sContext, ClrYellow); // yellow text
//        GrStringDraw(&sContext, str1, /*length*/ -1, /*x*/ 0, /*y*/ 0, /*opaque*/ false); //draw line 1
//        GrStringDraw(&sContext, str2, /*length*/ -1, /*x*/ 0, /*y*/ 10, /*opaque*/ false); //draw line 2 below line 1
//        GrFlush(&sContext); // flush the frame buffer to the LCD
    }
}
