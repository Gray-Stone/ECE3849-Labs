/*
 * GlobalSetting.c
 *
 *  Created on: 2019-4-7
 *      Author: leogr
 */

#include "globalSetting.h"
#include <stdint.h>
#include <stdbool.h>

struct Setting_Str
{
    uint16_t mVPerDiv  ; // set the voltage scale.
    uint32_t usPerDiv ; // set the time scale.
    char edgetype ;
    // maybe put the function names here to make it object-ish.
};

struct Setting_Str settings = {.mVPerDiv = 100 , .usPerDiv = 20 , .edgetype = 0 }  ;

void settingsReset()
{
    // add some checking about reentrant
    // take a semaphore
    settings.mVPerDiv = 100;
    settings.usPerDiv = 20;
    settings.edgetype = 0;
    // return a semaphore
}

bool changeVoltPerDiv(char direction )
{

    // semaphore check!
    // return false if can't access it.
    switch (settings.mVPerDiv)
    {
    case 100: settings.mVPerDiv = direction ? 200 : 100; break;
    case 200: settings.mVPerDiv = direction ? 500 : 100; break;
    case 500: settings.mVPerDiv = direction ? 1000 : 200; break;
    case 1000: settings.mVPerDiv = direction ? 1000 : 500;
    }
    return true
}

bool changeTimePerDiv(char direction )
{

//    switch (oldTimePerDiv)
//    {
//    case 20:
//        if (direction) {
//            newTimePerDiv = 50;
//            timerTriggerADC(400000);
//        }
//        break;
//    case 50:
//        if (direction) {
//            newTimePerDiv = 100;
//            timerTriggerADC(10000);
//        }
//        else {
//            newTimePerDiv = 20;
//            alwaysTriggerADC();
//        }
//        break;
//    case 100:
//        if (!direction) {
//            newTimePerDiv = 50;
//            timerTriggerADC(400000);
//        }
//        break;
//    }
//    return newTimePerDiv;
}
