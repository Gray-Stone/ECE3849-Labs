/*
 * screenControl.c
 *
 *  Created on: 2019-3-23
 *      Author: leogr
 */

#include"screenControl.h"

#include "Crystalfontz128x128_ST7735.h"



tContext sContext;
tRectangle rectFullScreen;


void screenInit()
{

	Crystalfontz128x128_Init(); // Initialize the LCD display driver
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP); // set screen orientation
    tRectangle rectFullScreenLocal = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1};
    rectFullScreen = rectFullScreenLocal;
}


void drawSamples( uint16_t * samplePointer , uint16_t length, uint16_t vPerDiv  )
{
	GrContextForegroundSet(&sContext, ClrBlack);
	GrRectFill(&sContext, &rectFullScreen); // fill screen with black

    char str1[50];   // string buffer line 1
    char str2[50];  //string buffer line 2


	GrContextForegroundSet(&sContext, ClrYellow); // yellow text
	GrStringDraw(&sContext, str1, /*length*/ -1, /*x*/ 0, /*y*/ 0, /*opaque*/ false); //draw line 1
	GrStringDraw(&sContext, str2, /*length*/ -1, /*x*/ 0, /*y*/ 10, /*opaque*/ false); //draw line 2 below line 1
	GrFlush(&sContext); // flush the frame buffer to the LCD

}
