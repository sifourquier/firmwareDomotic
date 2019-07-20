/*
 * triac.c
 *
 *  Created on: Nov 25, 2018
 *      Author: simon
 */

#include "triac.h"
#include "pwm.h"

#ifdef TRIACK

static volatile int power=MAX_POWER_TRIACK;
volatile int flagZeroCross=0;

void triacInit()
{
	init_timer16(1, 0xFFFF);

	NVIC_EnableIRQ(EINT_TRIAC);
	GPIOSetInterrupt(PORT_nZERO_CROSS,BIT_nZERO_CROSS, 0, 0, 0); //falin edge
	//GPIOIntEnable(PORT_nZERO_CROSS,BIT_nZERO_CROSS);
	GPIOPORT_nZERO_CROSS->IE |= (0x1<<BIT_nZERO_CROSS);
}

void set_triac(uint16_t l_power)
{
	if(l_power>MAX_POWER_TRIACK)
		l_power=MAX_POWER_TRIACK;
	power=l_power*10;
}

#define TIME_DISABLE 9800
#define TIME_DISABLE2 19800

void PIOINT0_IRQHandler()
{
	SET_BIT(PORT_TRIAC1,BIT_TRIAC1,0);
	flagZeroCross=1;
	//GPIOIntClear(PORT_nZERO_CROSS,BIT_nZERO_CROSS);
	GPIOPORT_nZERO_CROSS->IC |= (0x1<<BIT_nZERO_CROSS);

	if(power==MAX_POWER_TRIACK)
	{
		LPC_TMR16B1->TCR=2;
		SET_BIT(PORT_TRIAC1,BIT_TRIAC1,1);
	}
	else if(power==0)
	{
		LPC_TMR16B1->TCR=2;
		SET_BIT(PORT_TRIAC1,BIT_TRIAC1,0);
	}
	else
	{
		//GPIOIntDisable(PORT_nZERO_CROSS,BIT_nZERO_CROSS);//disable for nex 10ms (bounding)
		GPIOPORT_nZERO_CROSS->IE &= ~(0x1<<BIT_nZERO_CROSS);

		//GPIOIntClear(PORT_nZERO_CROSS,BIT_nZERO_CROSS);
		GPIOPORT_nZERO_CROSS->IC |= (0x1<<BIT_nZERO_CROSS);

		//SET_BIT(PORT_TRIAC1,BIT_TRIAC1,0);

		LPC_TMR16B1->MR0=TIME_DISABLE-60-power;
		LPC_TMR16B1->TCR=2;
		LPC_TMR16B1->TCR=1;
	}
}

void TIMER16_1_IRQHandler(void)
{
	if(LPC_TMR16B1->MR0<TIME_DISABLE)
	{
		SET_BIT(PORT_TRIAC1,BIT_TRIAC1,1);
		LPC_TMR16B1->MR0=TIME_DISABLE;
	}
	else if(LPC_TMR16B1->MR0==TIME_DISABLE)//seconde zero crossing
	{
		SET_BIT(PORT_TRIAC1,BIT_TRIAC1,0);
		LPC_TMR16B1->MR0=TIME_DISABLE2-power;
		//GPIOIntClear(PORT_nZERO_CROSS,BIT_nZERO_CROSS); //clear pending
		GPIOPORT_nZERO_CROSS->IC |= (0x1<<BIT_nZERO_CROSS);
		//GPIOIntEnable(PORT_nZERO_CROSS,BIT_nZERO_CROSS);
		GPIOPORT_nZERO_CROSS->IE |= (0x1<<BIT_nZERO_CROSS);
	}
	else if(LPC_TMR16B1->MR0<TIME_DISABLE2)
	{
		SET_BIT(PORT_TRIAC1,BIT_TRIAC1,1);
		LPC_TMR16B1->MR0=TIME_DISABLE2;
	}
	else
	{
		SET_BIT(PORT_TRIAC1,BIT_TRIAC1,0);
	}
	LPC_TMR16B1->IR = 0xFF;
}

#endif
