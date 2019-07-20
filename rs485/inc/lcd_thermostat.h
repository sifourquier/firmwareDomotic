/*
 * lcd_thermostat.h
 *
 *  Created on: 23 déc. 2018
 *      Author: simon
 */

#ifndef LCD_THERMOSTAT_H_
#define LCD_THERMOSTAT_H_

#include "ssp.h"
#include "board.h"
#include "utility.h"

#define BIG_EMPY 0x7FFF
#define SMALL_EMPY 0x7F

typedef struct LCD
{
	uint8_t week;
	uint8_t day1to7;
	uint8_t thermometer;
	uint8_t set;
	int16_t big_digit;
	uint8_t big_digit_hide_decimal;
	int16_t small_digit;
	uint8_t small_digit_hide_decimal;
	uint8_t home; //4bit 4picture of home
	uint8_t deg_c;
	uint8_t deg_c2;
	uint8_t rh;
	uint8_t point;
	int8_t small_digit_hour[2];
	uint8_t h;
	uint8_t twoPoints;
	uint8_t on;
	uint8_t off;
	uint8_t fan;
	uint8_t _auto;
	uint8_t heat;
	uint8_t unknow;
	uint8_t pm;
	uint8_t am;
	uint8_t snow;
	uint8_t sun;
	uint8_t fan_unknow;
	uint8_t moon;
	uint8_t leaf;
	uint8_t wifi;
	uint8_t hand;
	uint8_t key;
	uint8_t in;
	uint8_t out;
} t_lcd;

void lcdInit();
uint8_t lcdSet(t_lcd lcd);
uint8_t lcdMap();

#endif /* LCD_THERMOSTAT_H_ */
