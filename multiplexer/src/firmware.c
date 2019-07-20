#include "LPC11xx.h"
#include <cr_section_macros.h>
#include <NXP/crp.h>
#include <time.h>
#include "uart.h"
#include "iap_driver.h"
#include "modbus.h"
#include <stdio.h>


#define FLASH_SIZE (32*1024) //lpc 1114/333 = 56kB but try to use only 32kB

volatile uint32_t msTicks;
__attribute__ ((section(".after_vectors")))
void SysTick_Handler(void) {
	msTicks++;
}

__inline static void delay_100ms(uint32_t del)
{
	uint32_t curTicks;
	curTicks = msTicks;
	while ((msTicks - curTicks) < del)
	{
		//__WFI();
	}
}

__inline static void delay_s(uint32_t del)
{
	int i;
	for(i=0;i<del;i++)
	{
		delay_100ms(10);
	}
}

#define CMD_DATA 0
#define CMD_CRC 1

#define SECTORSIZE (4*1024)
void firmwareStore(int adresse,uint8_t* buf, int size)//size need a value = 256*2^x
{
	adresse+=(FLASH_SIZE/2);
	__disable_irq();
	if(adresse%SECTORSIZE==0)//nouvelle page
	{
		volatile int ret=iapPrepareSector(adresse/SECTORSIZE,adresse/SECTORSIZE);
		if(ret!=0)
		{
			printf("watch iap_driver.h error %d\n",ret);
		}
		ret=iapEraseSector(adresse/SECTORSIZE, adresse/SECTORSIZE);
		if(ret!=0)
		{
			printf("watch iap_driver.h error %d\n",ret);
		}
	}
	volatile int ret=iapPrepareSector(adresse/SECTORSIZE,adresse/SECTORSIZE);
	if(ret!=0)
	{
		printf("watch iap_driver.h error %d\n",ret);
	}
	ret=iapCopyRamToFlash((void*)buf,(void*)adresse,size);
	if(ret!=0)
	{
		printf("watch iap_driver.h error %d\n",ret);
	}
	volatile int test=*(int*)(adresse);
	__enable_irq();
}

__RAMFUNC(RAM) void firmwareFlash(volatile uint16_t crc)
{
	uint8_t buf[256];
	uint8_t* pt_flash=(uint8_t*)(FLASH_SIZE/2);
	uint16_t crc_flash=0xFFFF;
	modeBusCrc((uint8_t*)pt_flash,FLASH_SIZE/2,&crc_flash);
	if(crc_flash==crc)
	{
		__disable_irq();
		LPC_SYSCON->WDTCLKDIV=0; //desactive watchdog;
		iapPrepareSector(0,FLASH_SIZE/2/SECTORSIZE-1);
		iapEraseSector(0,FLASH_SIZE/2/SECTORSIZE-1);
		for(volatile int l=0;l<(FLASH_SIZE/2);l+=sizeof(buf))
		{
				if(l%(4*1024)==0)//nouvelle page
				{
					iapPrepareSector(l,l+1);
				}
				for(int l2=0;l2<sizeof(buf);l2++)
				{
					buf[l2]=*pt_flash;
					pt_flash++;
				}
				iapPrepareSector(l/SECTORSIZE,l/SECTORSIZE);
				iapCopyRamToFlash((void*)buf,(void*)l,sizeof(buf));
		}
		//volatile int l=0;
		//for(l=0;l<1000000;l++);

		NVIC_SystemReset();
		while(1);
	}
	else
	{
		printf("FIRMWARE CRC ERROR\n");
	}
}

/**
 * \brief decode a paquet of firmware format data[0]=cmd 0=data 1crc and end, data[1]<<8|data[2]= adresse in flash; data[3]<<8|data[4]= size of paquet; data[x+5]..data[x+5+2^n] = firmware
 */
void firmwareDecode(uint8_t * data,uint16_t nbByte)
{
	int adresse;
	uint16_t crc;
	uint8_t cmd=data[0];
	switch (cmd)
	{
	case CMD_DATA:
		adresse=(uint16_t)data[1]<<8|data[2];
		firmwareStore(adresse,data+3,nbByte-3);

		break;
	case CMD_CRC:
		crc=(uint16_t)data[1]<<8|data[2];
		firmwareFlash(crc);
	}
}
