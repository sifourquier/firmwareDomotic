/*
 * chargeur_v2.h
 *
 *  Created on: 27 avr. 2016
 *      Author: Arthur
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct
{
	uint32_t timeOfDay;
	uint8_t dayOfWeek;
	uint8_t dayOfMonth;
	uint8_t month;
	uint16_t year;
} t_Time;

//all in 32bit for alignement
typedef struct
{
	uint32_t modeBusaddress;
	uint32_t backlightPower;
} t_Config;

extern volatile t_Config config;

#define FLASH_SIZE (32*1024)
#define NB_TRIAC 1
#define NB_PWM 3
#define NB_OUT (NB_TRIAC+NB_PWM)
#define NB_LED_SWITCH 4

extern uint32_t SystemCoreClock;

void configSave();

#include "LPC11xx.h"

#endif /* MAIN_H_ */
