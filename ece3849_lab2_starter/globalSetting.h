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

struct Setting_Str
{
    uint16_t mVPerDiv  ; // set the voltage scale.
    uint32_t usPerDiv ; // set the time scale.
    char edge ;
    uint16_t triggerLevel ;
    // gate name is called settingGate
};


extern struct Setting_Str settings ;

void settingsReset();
bool changeVoltPerDiv(char direction );
bool changeTimePerDiv(char direction );
bool changeTriggerEdge();



#endif /* GLOBALSETTING_H_ */
