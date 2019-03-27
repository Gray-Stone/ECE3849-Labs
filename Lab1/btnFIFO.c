/*
 * btnFIFO.c
 *
 *  Created on: 2019-3-25
 *      Author: Leo
 */

#include "btnFIFO.h"



volatile uint32_t btnFIFO[FIFO_SIZE] = {0};
volatile unsigned char fifoRear =0;
volatile unsigned char fifoFront =0;

unsigned char fifoPut(uint32_t data)
{
    unsigned char newRear = fifoRear+1 ;
    if (newRear  >= FIFO_SIZE )
        newRear = 0;
    if ( newRear == fifoFront )
        return 0; // this is full
    btnFIFO[newRear] = data ;
    fifoRear = newRear;
    return 1;

}


uint32_t fifoPoll()
{
    if (fifoFront == fifoRear )
        return 0;

 
    if (fifoFront >= FIFO_SIZE-1 )
        fifoFront =0;
    else
        ++fifoFront ;
    return btnFIFO[ fifoFront ];
} 
