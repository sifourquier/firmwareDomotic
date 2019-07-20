#include "LPC11xx.h"

#include <cr_section_macros.h>

#include "modbus.h"
#include "main.h"
#include "board.h"
#include "DS18B20.h"
#include "pwm.h"
#include "oneWire.h"
#include "TLI4970.h"
#include "triac.h"

#define VERSION_CODE 3


uint32_t SystemCoreClock;

typedef enum
{
	Sds_START,
	Sds_READ,
	Sds_END
} t_stateReadDs18B20;

static int64_t ID_DS18B20=0x1213141516171819;
static int16_t pressure=9800;
static int16_t humidity=6500;
static int16_t temperature_DS18B20=2000;
static int16_t temperature_ext=2100;
static int16_t out[NB_OUT]={500,[1 ... NB_OUT-1]=MAX_POWER};

void configClock()
{
	LPC_SYSCON->SYSPLLCLKSEL  = 0;   /* Select PLL Input = IRC*/
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;               /* Update Clock Source      */
	LPC_SYSCON->SYSPLLCLKUEN  = 0x00;               /* Toggle Update Register   */
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;
	while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));     /* Wait Until Updated       */
	LPC_SYSCON->SYSPLLCTRL    = 0x03; //P=(2^0) PLL M=(3+1)
	LPC_SYSCON->PDRUNCFG     &= ~(1 << 7);          /* Power-up SYSPLL          */
	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));           /* Wait Until PLL Locked    */

	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));
	LPC_SYSCON->MAINCLKSEL    = 0x03;     			/* Select PLL Clock for main Clock  */
	LPC_SYSCON->MAINCLKUEN    = 0x01;               /* Update MCLK Clock Source */
	LPC_SYSCON->MAINCLKUEN    = 0x00;               /* Toggle Update Register   */
	LPC_SYSCON->MAINCLKUEN    = 0x01;
	while (!(LPC_SYSCON->MAINCLKUEN & 0x01));       /* Wait Until Updated       */

	LPC_SYSCON->SYSAHBCLKDIV  = 1; //clock divider =1

	SystemCoreClock=48000000;
}

void modBusCallBackWrite(t_Function function,uint32_t adresse,uint32_t value)
{
	if(function == F_WRITE_COIL || function == F_WRITE_COILS)
		if(value)
			value=MAX_POWER;
	if(adresse<NB_OUT)
	{
		out[adresse]=value;
		if(adresse<NB_TRIAC)
		{
			set_triac(value);
		}
		else
		{
			setPwm(adresse-NB_TRIAC,value);
		}
	}
}

int modBusCallBackRead(t_Function function,uint32_t adresse,uint8_t* data) //return number of bit of data
{
	const uint8_t PORT_SW[]={LIST_PORT_SW};
	const uint8_t BIT_SW[]={LIST_BIT_SW};
	if(function==F_READ_INPUT || function==F_READ_INPUT_REG) //read input
	{
		if(adresse<sizeof(PORT_SW))
		{
			if(function==F_READ_INPUT)
			{
				data[0]&=~(1<<adresse);
				data[0]|=(!GET_BIT(PORT_SW[adresse],BIT_SW[adresse])<<adresse);

				return 1; //return size in bit
			}
			else if(function==F_READ_INPUT_REG)
			{
				if(!GET_BIT(PORT_SW[adresse],BIT_SW[adresse]))
				{
					data[0]=0xFF;
					data[1]=0xFF;
				}else{
					data[0]=0x00;
					data[1]=0x00;
				}
				return 16; //return size in bit
			}
		}
		if(function==F_READ_INPUT_REG)
		{
			if(adresse==sizeof(PORT_SW))
			{
				data[0]=temperature_DS18B20>>8;
				data[1]=temperature_DS18B20;
				return 16; //return size in bit
			}
			else if(adresse==sizeof(PORT_SW)+1)
			{
				data[0]=temperature_ext>>8;
				data[1]=temperature_ext;
				return 16; //return size in bit
			}
			else if(adresse==sizeof(PORT_SW)+2)
			{
				data[0]=humidity>>8;
				data[1]=humidity;
				return 16; //return size in bit
			}
			else if(adresse==sizeof(PORT_SW)+3)
			{
				data[0]=pressure>>8;
				data[1]=pressure;
				return 16; //return size in bit
			}
			else if(adresse>=sizeof(PORT_SW)+4 && adresse<sizeof(PORT_SW)+4+4)
			{
				int16_t IDtemp=ID_DS18B20>>(16*(adresse-(sizeof(PORT_SW)+4)));
				data[0]=IDtemp>>8;
				data[1]=IDtemp;
				return 16; //return size in bit
			}
			else if(adresse==1000)
			{
				data[0]=VERSION_CODE>>8;
				data[1]=VERSION_CODE;
				return 16; //return size in bit
			}
		}
	}
	else //read outpout
	{
		if(adresse<NB_OUT)
		{
			if(function==F_READ_COIL)
			{
				data[0]&=~(1<<adresse);
				data[0]|=(out[adresse]>0)<<adresse;

				return 1; //return size in bit
			}
			else if(function==F_READ_REG)
			{
				data[0]=out[adresse]>>8;
				data[1]=out[adresse];
				return 16;//return size in bit
			}
		}
	}
	return 0;

}

#define GPIO_RX LPC_GPIO1
#define GPIO_TX LPC_GPIO1

#define MASK_GPIO (0x55<<2)

#define GPIO_OUT LPC_GPIO0
#define MASK_GPIO (0x55<<2)

#define MAX_PAUSE 10000 //2ms

int main(void) {

	LPC_SYSCON->SYSAHBCLKCTRL |= (0x1UL << 16); //power IOCON
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6); //power gpio
	configClock();
	initBoard();

	while(1)
	{
		GPIO_RX->DIR=~MASK_GPIO;
		GPIO_OUT->DIR=~MASK_GPIO|2;
		uint32_t allRX;
		uint32_t maskCurentIn;
		uint32_t oldMaskCurentIn;
		uint32_t timeout;
		allRX=GPIO_RX->DATA&MASK_GPIO;
		if(allRX!=MASK_GPIO)
		{
			GPIO_OUT->DATA=~allRX; //the data have receive data stay in input all other as out
			maskCurentIn=~allRX&MASK_GPIO; //get a mask for only the current in
			timeout=0;
			while(timeout<MAX_PAUSE)
			{
				timeout++;
				GPIO_OUT->DATA=~GPIO_OUT->DATA;
				if(maskCurentIn)
				{
					GPIO_TX->DATA=~MASK_GPIO;
				}
				else
				{
					GPIO_TX->DATA=MASK_GPIO;
				}
				if(oldMaskCurentIn!=maskCurentIn)
					oldMaskCurentIn=maskCurentIn;
			}
		}
	}


}
