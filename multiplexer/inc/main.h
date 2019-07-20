/*
 * chargeur_v2.h
 *
 *  Created on: 27 avr. 2016
 *      Author: Arthur
 */

#ifndef CHARGEUR_H_
#define CHARGEUR_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define FLASH_SIZE (32*1024)
#define NB_TRIAC 1
#define NB_PWM 3
#define NB_OUT (NB_TRIAC+NB_PWM)

extern uint32_t SystemCoreClock;

#include "LPC11xx.h"

#endif /* CHARGEUR_H_ */
