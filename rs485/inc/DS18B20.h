/*
 * DS18B20.h
 *
 *  Created on: Nov 16, 2018
 *      Author: simon
 */

#ifndef DS18B20_H_
#define DS18B20_H_

void initDS18B20();
int16_t getAndRunDS18B20();
uint64_t getIdDS18B20(); //locking fuction*/
int8_t RunDS18B20();
int8_t readDS18B20();
int8_t getDS18B20(volatile int16_t* t);

#endif /* DS18B20_H_ */
