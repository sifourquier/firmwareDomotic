/*
 * sht30.h
 *
 *  Created on: 23 déc. 2018
 *      Author: simon
 */

#ifndef SHT30_H_
#define SHT30_H_

typedef struct
{
	int temperature;
	int humidity;
} sht30Data;

void sht30Init();
sht30Data sht30Read();

#endif /* SHT30_H_ */
