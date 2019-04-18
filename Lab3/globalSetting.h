/*
 * GlobalSetting.h
 *
 *  Created on: 2019-4-7
 *      Author: leogr
 */

#ifndef GLOBALSETTING_H_
#define GLOBALSETTING_H_

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <xdc/cfg/global.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>


#include <stdbool.h>

// this need to be defined here for accessing.
// read is direct access. write should go though fuctions.
struct Setting_Str
{
    uint16_t mVPerDiv  ; // set the voltage scale.
    uint32_t usPerDiv ; // set the time scale.
    char edge ;
    uint16_t triggerLevel ;
    char FFT;
    // gate name is called settingGate
    char DMA_Software; // 0 for Software, 1 for DMA     // flag to check which way to get ADC pointer.

};

// make it external accessable
extern struct Setting_Str settings ;

// functions to change contents, these all have built in gates.
void settingsReset();
bool changeVoltPerDiv(char direction );
bool changeTimePerDiv(char direction );
bool changeTriggerEdge();
bool changeFFTMode();



#endif /* GLOBALSETTING_H_ */
