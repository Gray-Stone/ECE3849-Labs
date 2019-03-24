/*
 * screenControl.c
 *
 *  Created on: 2019-3-23
 *      Author: leogr
 */

#include"screenControl.h"
#include "sampler.h" //needed for defines
#include <math.h>

#include "Crystalfontz128x128_ST7735.h"



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


void drawSamples( uint16_t * samplePointer , uint16_t length, uint16_t mVPerDiv)
{
	GrContextForegroundSet(&sContext, ClrBlack);
	GrRectFill(&sContext, &rectFullScreen); // fill screen with black

//    char str1[50];   // string buffer line 1
//    char str2[50];  //string buffer line 2


	GrContextForegroundSet(&sContext, ClrYellow); // yellow text

	//FOR TESTING::
	float fVoltsPerDiv = ((float)mVPerDiv)/1000;
	//

	int x, y;
	float fScale = (VIN_RANGE * PIXELS_PER_DIV)/((1 << ADC_BITS) * fVoltsPerDiv);
	for (x = 0; x < 128; x++){
	    y = LCD_VERTICAL_MAX/2 - (int)roundf(fScale * ((int)samplePointer[x] - ADC_OFFSET));
	    GrPixelDraw(&sContext, x, y);
	}
//	GrStringDraw(&sContext, str1, /*length*/ -1, /*x*/ 0, /*y*/ 0, /*opaque*/ false); //draw line 1
//	GrStringDraw(&sContext, str2, /*length*/ -1, /*x*/ 0, /*y*/ 10, /*opaque*/ false); //draw line 2 below line 1
	GrFlush(&sContext); // flush the frame buffer to the LCD

}
