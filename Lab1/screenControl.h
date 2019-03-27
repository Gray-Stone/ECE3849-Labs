/*
 * screenControl.h
 *
 *  Created on: 2019-3-23
 *      Author: leogr
 */

#ifndef SCREENCONTROL_H_
#define SCREENCONTROL_H_

#include <stdint.h>
#include <stdbool.h>

#define PIXELS_PER_DIV 20 //LCD pixels per voltage division

void screenInit();
void drawScreen( uint16_t * samplePointer , uint16_t length, uint16_t vPerDiv , char edgetype, float cpu_load);




#endif /* SCREENCONTROL_H_ */
