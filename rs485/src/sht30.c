/*
 * sht30.c
 *
 *  Created on: 23 déc. 2018
 *      Author: simon
 */

#include "sht30.h"
#include "i2c.h"

#ifdef BOARD_THERMOSTAT
	#define SHT30_ADD (0x45<<1)
#else
	#define SHT30_ADD (0x44<<1)
#endif

void sht30Init()
{
	unsigned char config[2] = {0x21,0x30};

	I2CInit();
	i2cWriteRead(SHT30_ADD, config, sizeof(config),0,0);
}

sht30Data sht30Read()
{
	unsigned char cmd[2] = {0xE0,0x00};
	sht30Data val;
	unsigned char  data[6];//can be change size to 5? no read last crc
	i2cWriteRead(SHT30_ADD, cmd,sizeof(cmd),data, sizeof(data));
	int temp = ((data[0]<<8) | data[1]);
	val.temperature = (-4500 + (17500 * temp / 65535));
	val.humidity = 10000 * ((data[3]<<8) | data[4]) / 65535;
	return val;
}
