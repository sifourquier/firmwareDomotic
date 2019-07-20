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

void init_triac();
void set_triac(uint16_t l_power);

#endif /* TRIAC_H_ */
