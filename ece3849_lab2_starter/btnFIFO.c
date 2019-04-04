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

// push the data into the FIFO, return 1 for sucess, return 0 for fifo full.
unsigned char fifoPut(uint32_t data)
{
    unsigned char newRear = fifoRear+1 ;
    if (newRear  >= FIFO_SIZE ) // warp around case
        newRear = 0;
    if ( newRear == fifoFront ) // this is full
        return 0;
    btnFIFO[newRear] = data ; // push the data in
    fifoRear = newRear; // update the pointer
    return 1;

}

// get the last item from fifo and remove the item from list, return the value, 0 for no last item.
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
