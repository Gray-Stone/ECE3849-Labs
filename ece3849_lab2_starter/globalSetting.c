/*
 * globalSetting.c
 *
 *  Created on: 2019-4-7
 *      Author: leogr
 */


#include "globalSetting.h"
#include <stdint.h>
#include <stdbool.h>

 #include "sampler.h"


 struct Setting_Str settings = {.mVPerDiv = 100 , .usPerDiv = 20 , .edge = 0 , .triggerLevel = ADC_OFFSET  }  ;

 void settingsReset()
{
    static IArg keySettingGate;
    keySettingGate = GateMutex_enter(settingGate);

     settings.mVPerDiv = 500 ;
    settings.usPerDiv = 20 ;
    settings.edge = 0 ;
    settings.triggerLevel = ADC_OFFSET;
    GateMutex_leave(settingGate, keySettingGate);

 }

 bool changeVoltPerDiv(char direction )
{
    static IArg keySettingGate;
    keySettingGate = GateMutex_enter(settingGate);
    // semaphore check!
    // return false if can't access it.
    switch (settings.mVPerDiv)
    {
    case 100: settings.mVPerDiv = direction ? 200 : 100; break;
    case 200: settings.mVPerDiv = direction ? 500 : 100; break;
    case 500: settings.mVPerDiv = direction ? 1000 : 200; break;
    case 1000: settings.mVPerDiv = direction ? 1000 : 500; break;
    default : 500
    }

     GateMutex_leave(settingGate, keySettingGate);
    return true;
}

 bool changeTriggerEdge()
{
    static IArg keySettingGate;
    keySettingGate = GateMutex_enter(settingGate);

     settings.edge^=0x01 ;

     GateMutex_leave(settingGate, keySettingGate);

}


 bool changeTimePerDiv(char direction )
{
    static IArg keySettingGate;
    keySettingGate = GateMutex_enter(settingGate);


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
    GateMutex_leave(settingGate, keySettingGate);
}
