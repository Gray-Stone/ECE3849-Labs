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
#include <stdbool.h>
#include <string.h>
#include "Crystalfontz128x128_ST7735.h"

#include <math.h>
#include "kiss_fft.h"
#include "_kiss_fft_guts.h"
#define PI 3.14159265358979f
#define NFFT 1024 // FFT length
#define KISS_FFT_CFG_SIZE (sizeof(struct kiss_fft_state)+sizeof(kiss_fft_cpx)*(NFFT-1))


#include "globalSetting.h"

tContext sContext;
tRectangle rectFullScreen;

int processedWaveform[SCREENSIZE];
volatile bool processedFlag = false ; // false is good for write. True is good for read



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
    unsigned char x =0;
    float fVoltsPerDiv ;
    float fScale ;
    char kiss_fft_cfg_buffer[KISS_FFT_CFG_SIZE];// Kiss FFT config memory
    size_t buffer_size = KISS_FFT_CFG_SIZE;
    kiss_fft_cfg cfg;               //   Kiss FFT config
    kiss_fft_cpx in[NFFT], out[NFFT]; //   complex waveform and spectrum buffers
    int i;

    cfg = kiss_fft_alloc(NFFT, 0, kiss_fft_cfg_buffer, &buffer_size);// init Kiss FFT

    while(1)
    {
        Semaphore_pend(processingSem,BIOS_WAIT_FOREVER);

        if (settings.FFT) {
            for(i = 0; i < NFFT; i++) {// generate an input waveform
                in[i].r = FFTBuffer[i]; // real part of waveform
                in[i].i = 0; // imaginary part of waveform
             }
            kiss_fft(cfg, in, out);      // compute FFT
            // convert first 128 bins of out[] to dB for display
         }
        else {
            processedFlag = false ; // there should never be a case of processing is started and flag is true.

            fVoltsPerDiv = ((float) (settings.mVPerDiv) )/1000;
            fScale = (VIN_RANGE * PIXELS_PER_DIV)/((1 << ADC_BITS) * fVoltsPerDiv);
            for(x=0; x<SCREENSIZE ; ++x )
            {
                processedWaveform[x] = LCD_VERTICAL_MAX/2 - (int)roundf(fScale * ((int)waveformBuffer[x] - ADC_OFFSET));
            }

            processedFlag= true;        // mark the buffer is holding newer content.
        }

        Semaphore_post(triggerFindSem);
        Semaphore_post(displaySem);
    }

}

void DisplayTask(UArg arg1, UArg arg2) //6
{
//    tContext sContext;
//    tRectangle rectFullScreen;
//
//    Crystalfontz128x128_Init(); // Initialize the LCD display driver
//    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP); // set screen orientation
//    GrContextInit(&sContext, &g_sCrystalfontz128x128); // Initialize the grlib graphics context
//    tRectangle rectFullScreenLocal = {0, 0, GrContextDpyWidthGet(&sContext)-1, GrContextDpyHeightGet(&sContext)-1};
//    rectFullScreen = rectFullScreenLocal;
//    GrContextFontSet(&sContext, &g_sFontFixed6x8); // select font

    int localWaveform[SCREENSIZE];

    int i, pastY;

    while(1) {
        Semaphore_pend(displaySem,BIOS_WAIT_FOREVER);

        if (processedFlag) {
            for(i = 0; i < SCREENSIZE; i++) {
                localWaveform[i] = processedWaveform[i];
            }
            processedFlag = false;
        }


        GrContextForegroundSet(&sContext, ClrBlack);
        GrRectFill(&sContext, &rectFullScreen); // fill screen with black background
        GrContextForegroundSet(&sContext, ClrBlue); //blue grid lines

        //draw grid
        for (i = 3; i < 128; i+= PIXELS_PER_DIV) {
            GrLineDraw(&sContext, i, 0, i, 127); //vertical lines
            GrLineDraw(&sContext, 0, i, 127, i); //horizontal lines
        }

        GrContextForegroundSet(&sContext, ClrYellow); // yellow for samples
        for (i = 0; i < 128; i++){
            if (i == 0){
                GrPixelDraw(&sContext, i, localWaveform[i]);
            }
            else {
                GrLineDraw(&sContext, i - 1, pastY, i, localWaveform[i]);
            }
            pastY = localWaveform[i];
        }
        GrContextForegroundSet(&sContext, ClrWhite); //white text
        char str1[50];   // string buffer line 1
        char str2[50];  //string buffer line 2
        char edgeString[10]; //string buffer for edge display string
        char voltString[10]; //string buffer for voltage scale display string

        if (settings.mVPerDiv == 1000)
            strcpy(voltString, "  1  V");
        else
            snprintf(voltString, 10, "%u mV\0", settings.mVPerDiv);
        if (settings.edge == 0)
            strcpy(edgeString, "rise");
        else
            strcpy(edgeString, "fall");

        snprintf(str1, 50, "%u uS  %s %s\0", 20, voltString, edgeString); //Settings status bar
        snprintf(str2, 50, "CPU Load: %.5f%", 0);//cpu_load*100); //Settings status bar
        GrStringDraw(&sContext, str1, /*length*/ -1, /*x*/ 0, /*y*/ 0, /*opaque*/ false); //draw top bar
        GrStringDraw(&sContext, str2, /*length*/ -1, /*x*/ 0, /*y*/ 120, /*opaque*/ false); //draw line 2 below line 1

        GrFlush(&sContext); // flush the frame buffer to the LCD

    }

}











//void drawScreen( uint16_t * samplePointer , uint16_t length, uint16_t mVPerDiv, char edgetype, float cpu_load)
//{
//
//	GrContextForegroundSet(&sContext, ClrBlack);
//	GrRectFill(&sContext, &rectFullScreen); // fill screen with black background
//	GrContextForegroundSet(&sContext, ClrBlue); //blue grid lines
//	int i;
//	//draw grid
//	for (i = 3; i < 128; i+= PIXELS_PER_DIV) {
//	    GrLineDraw(&sContext, i, 0, i, 127); //vertical lines
//	    GrLineDraw(&sContext, 0, i, 127, i); //horizontal lines
//	}
//
//	GrContextForegroundSet(&sContext, ClrYellow); // yellow for samples
//
//	//FOR TESTING::
//	float fVoltsPerDiv = ((float)mVPerDiv)/1000;
//
//	int x, y, pastY;
//	float fScale = (VIN_RANGE * PIXELS_PER_DIV)/((1 << ADC_BITS) * fVoltsPerDiv);
//	for (x = 0; x < 128; x++){
//	    y = LCD_VERTICAL_MAX/2 - (int)roundf(fScale * ((int)samplePointer[x] - ADC_OFFSET));
//	    if (x == 0){
//	        GrPixelDraw(&sContext, x, y);
//	    }
//	    else {
//	        GrLineDraw(&sContext, x - 1, pastY, x, y);
//	    }
//        pastY = y;
//	}
//	GrContextForegroundSet(&sContext, ClrWhite); //white text
//	char str1[50];   // string buffer line 1
//	char str2[50];  //string buffer line 2
//	char edgeString[10]; //string buffer for edge display string
//	char voltString[10]; //string buffer for voltage scale display string
//
//	if (mVPerDiv == 1000)
//	    strcpy(voltString, "  1  V");
//	else
//	    snprintf(voltString, 10, "%u mV\0", mVPerDiv);
//	if (edgetype == 0)
//	    strcpy(edgeString, "rise");
//	else
//	    strcpy(edgeString, "fall");
//
//    snprintf(str1, 50, "%u uS  %s %s\0", 20, voltString, edgeString); //Settings status bar
//    snprintf(str2, 50, "CPU Load: %.5f%", cpu_load*100); //Settings status bar
//	GrStringDraw(&sContext, str1, /*length*/ -1, /*x*/ 0, /*y*/ 0, /*opaque*/ false); //draw top bar
//	GrStringDraw(&sContext, str2, /*length*/ -1, /*x*/ 0, /*y*/ 120, /*opaque*/ false); //draw line 2 below line 1
//
//	GrFlush(&sContext); // flush the frame buffer to the LCD
//
//}

