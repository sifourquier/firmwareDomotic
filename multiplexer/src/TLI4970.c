#include "ssp.h"
#include "stdint.h"
#include "board.h"

#define TIME_TRIAC

void initTLI4970()
{
	SSP_Init(0);
}

int readTLI4970() //return current in ma
{
	uint8_t buff[2];
	do
	{
		SET_BIT(PORT_nCS_TLI,BIT_nCS_TLI,0);
		SSP_Receive(0,buff,sizeof(buff));
		SET_BIT(PORT_nCS_TLI,BIT_nCS_TLI,1);
	} while(buff[0]&(1<<7)); //if Status=1 read seconde time

	int current = (int)(((int)(buff[0]&0x1F)<<8)|buff[1])-4096;
	current=current*1000/80;
	return current;
}
