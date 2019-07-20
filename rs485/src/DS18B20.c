/*
 * DS18B20.c
 *
 *  Created on: Nov 16, 2018
 *      Author: simon
 */
#include "stdint.h"
#include "oneWire.h"

#define DS18B20_START 0x44
#define DS18B20_WRITE 0x4E
#define DS18B20_READ 0xBE
#define DS18B20_READ_ID 0x33 /*READ ROM*/

#define NULL 0

void initDS18B20()
{
	initOneWire();
	uint8_t send[]={ONE_WIRE_SKIP_ROM,DS18B20_WRITE,0xFF,0x00,3<<5}; //no alarm 9+1bit
	SendReadOneWire(send,sizeof(send),0);
	while(getDataOneWire(NULL)==-1);
}

uint64_t getIdDS18B20()
{
	uint64_t temp;
	uint8_t send[]={DS18B20_READ_ID};
	uint8_t* read;
	SendReadOneWire(send,sizeof(send),8);
	while(getDataOneWire(&read)==-1);
	temp=(uint64_t)(read[0]|((uint64_t)read[1]<<8)|((uint64_t)read[2]<<16)|((uint64_t)read[3]<<24)
			|((uint64_t)read[4]<<32)|((uint64_t)read[5]<<40)|((uint64_t)read[6]<<48)|((uint64_t)read[7]<<56));
	return temp;
}

int8_t RunDS18B20()
{
	uint8_t send[]={ONE_WIRE_SKIP_ROM,DS18B20_START};
	return SendReadOneWire(send,sizeof(send),0); //return -1 if do not can start OneWire
}

int8_t readDS18B20()
{
	uint8_t send[]={ONE_WIRE_SKIP_ROM,DS18B20_READ};
	return SendReadOneWire(send,sizeof(send),2);
}

int8_t getDS18B20(volatile int16_t* t)
{
	uint8_t* read;
	if(getDataOneWire(&read)>-1)
	{
		*t=(int)(read[0]|((short)read[1]<<8))*100/16;
		return 1;
	}
	else
	{
		return -1;
	}

}

int16_t getAndRunDS18B20()
{
	int16_t temp;
	{
		uint8_t send[]={ONE_WIRE_SKIP_ROM,DS18B20_READ};
		uint8_t* read;
		SendReadOneWire(send,sizeof(send),2);
		while(getDataOneWire(&read)==-1);
		temp=(int)(read[0]|((short)read[1]<<8))*100/16;
	}
	{
		uint8_t send[]={ONE_WIRE_SKIP_ROM,DS18B20_START};
		SendReadOneWire(send,sizeof(send),0);
		while(getDataOneWire(NULL)==-1);
	}
	return temp;
}
