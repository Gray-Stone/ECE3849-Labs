/*
 * btnFIFO.c
 *
 *  Created on: 2019Äê3ÔÂ25ÈÕ
 *      Author: Leo
 */

#include "btnFIFO.h"
#include <stdint.h>



volatile uint32_t btnFIFO[FIFO_SIZE] = {0};
volatile unsigned char fifoRear =0;
volatile unsigned char fifoFront =0;

unsigned char fifoPut(uint32_t data)
{
    unsigned char newRaer = fifoRear+1 ;
    if (newRaer >= FIFO_SIZE )
        newRaer = 0;
    if ( newRear == fifoFront )
        return 0; // this is full
    btnFIFO[newRear] = data ;
    return 1;

}


uint32_t fifoPoll()
{
    if (fifoFront == fifoRear )
        return 0;

    static uin32_t tempData;
    tempData = btnFIFO[ fifoFront ] ;
    if (fifoFront >= FIFO_SIZE-1 )
        fifoFront =0;
    else
        ++fifoFront ;
    return tempData;
}
