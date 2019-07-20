/*
 * triac.h
 *
 *  Created on: Nov 25, 2018
 *      Author: simon
 */

#ifndef TRIAC_H_
#define TRIAC_H_

#include "board.h"
#include "timer16.h"
#include "gpio.h"
#include "pwm.h"
#define MAX_POWER_TRIACK 1000


void triacInit();
void set_triac(uint16_t l_power);

extern volatile int flagZeroCross;

#endif /* TRIAC_H_ */
