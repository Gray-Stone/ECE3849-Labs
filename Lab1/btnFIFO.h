/*
 * btnFIFO.h
 *
 *  Created on: 2019Äê3ÔÂ25ÈÕ
 *      Author: Leo
 */

#ifndef BTNFIFO_H_
#define BTNFIFO_H_


#define FIFO_SIZE 10

#include <stdint.h>

unsigned char fifoPut(uint32_t data);
uint32_t fifoPoll();


#endif /* BTNFIFO_H_ */
