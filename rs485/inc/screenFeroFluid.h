/*
 * screen.h
 *
 *  Created on: 20 mars 2019
 *      Author: simon
 */

#include "stdint.h"

#ifndef SCREEN_H_
#define SCREEN_H_

#define WIDTH 17
#define HEIGHT 6

void screenInit();
void printTime(uint8_t h,uint8_t m,uint8_t s);
void screenBare();

#endif /* SCREEN_H_ */
