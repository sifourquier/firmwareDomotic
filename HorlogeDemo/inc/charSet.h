/*
 * charSet.h
 *
 *  Created on: 19 mars 2019
 *      Author: simon
 */

#include "stdint.h"
#include "screenFeroFluid.h"

#ifndef CHARSET_H_
#define CHARSET_H_

void writeChar(int8_t screen[WIDTH][HEIGHT], int pos, uint8_t c, int power);

#endif /* CHARSET_H_ */
