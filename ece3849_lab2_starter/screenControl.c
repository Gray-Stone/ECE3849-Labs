/*
 * screenControl.c
 *
 *  Created on: 2019-3-23
 *      Author: leogr
 */

#include"screenControl.h"
#include "sampler.h" //needed for samples2Draw
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "Crystalfontz128x128_ST7735.h"
#include "globalSetting.h"

tContext sContext;
tRectangle rectFullScreen;

void screenInit()
{
	Crystalfontz128x128_Init(); // Initialize the LCD display driver
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP); // set screen orientation
    GrContextInit(&sContext, &g_sCrystalfontz128x128); // Initialize the grlib graphics context
    tRectangle rectFullScreenLocal = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1};
    rectFullScreen = rectFullScreenLocal;
    GrContextFontSet(&sContext, &g_sFontFixed6x8); // select font
}

void ProcessingTask(UArg arg1, UArg arg2) { //4

    Semaphore_pend(processingSem,BIOS_WAIT_FOREVER);

    samples2Draw;


    Semaphore_post(triggerFindSem);
    Semaphore_post(displaySem);

}

void DisplayTask(UArg arg1, UArg arg2) //6
{
    Semaphore_pend(displaySem,BIOS_WAIT_FOREVER);


}











void drawScreen( uint16_t * samplePointer , uint16_t length, uint16_t mVPerDiv, char edgetype, float cpu_load)
{
	GrContextForegroundSet(&sContext, ClrBlack);
	GrRectFill(&sContext, &rectFullScreen); // fill screen with black background
	GrContextForegroundSet(&sContext, ClrBlue); //blue grid lines
	int i;
	//draw grid
	for (i = 3; i < 128; i+= PIXELS_PER_DIV) {
	    GrLineDraw(&sContext, i, 0, i, 127); //vertical lines
	    GrLineDraw(&sContext, 0, i, 127, i); //horizontal lines
	}

	GrContextForegroundSet(&sContext, ClrYellow); // yellow for samples

	//FOR TESTING::
	float fVoltsPerDiv = ((float)mVPerDiv)/1000;

	int x, y, pastY;
	float fScale = (VIN_RANGE * PIXELS_PER_DIV)/((1 << ADC_BITS) * fVoltsPerDiv);
	for (x = 0; x < 128; x++){
	    y = LCD_VERTICAL_MAX/2 - (int)roundf(fScale * ((int)samplePointer[x] - ADC_OFFSET));
	    if (x == 0){
	        GrPixelDraw(&sContext, x, y);
	    }
	    else {
	        GrLineDraw(&sContext, x - 1, pastY, x, y);
	    }
        pastY = y;
	}
	GrContextForegroundSet(&sContext, ClrWhite); //white text
	char str1[50];   // string buffer line 1
	char str2[50];  //string buffer line 2
	char edgeString[10]; //string buffer for edge display string
	char voltString[10]; //string buffer for voltage scale display string

	if (mVPerDiv == 1000)
	    strcpy(voltString, "  1  V");
	else
	    snprintf(voltString, 10, "%u mV\0", mVPerDiv);
	if (edgetype == 0)
	    strcpy(edgeString, "rise");
	else
	    strcpy(edgeString, "fall");

    snprintf(str1, 50, "%u uS  %s %s\0", 20, voltString, edgeString); //Settings status bar
    snprintf(str2, 50, "CPU Load: %.5f%", cpu_load*100); //Settings status bar
	GrStringDraw(&sContext, str1, /*length*/ -1, /*x*/ 0, /*y*/ 0, /*opaque*/ false); //draw top bar
	GrStringDraw(&sContext, str2, /*length*/ -1, /*x*/ 0, /*y*/ 120, /*opaque*/ false); //draw line 2 below line 1

	GrFlush(&sContext); // flush the frame buffer to the LCD

}
