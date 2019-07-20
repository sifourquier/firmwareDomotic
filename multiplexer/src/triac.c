/*
 * triac.c
 *
 *  Created on: Nov 25, 2018
 *      Author: simon
 */

#include "triac.h"

static int power=MAX_POWER;

void triacInit()
{
	init_timer16(1, 0xFFFF);
	NVIC_EnableIRQ(EINT_TRIAC);
	GPIOSetInterrupt(PORT_nZERO_CROSS,BIT_nZERO_CROSS, 0, 0, 1);
	GPIOIntEnable(PORT_nZERO_CROSS,BIT_nZERO_CROSS);
}

void set_triac(uint16_t l_power)
{
	if(l_power>MAX_POWER)
		l_power=MAX_POWER-1;
	power=l_power*10;
}

void PIOINT0_IRQHandler()
{
	GPIOIntClear(PORT_nZERO_CROSS,BIT_nZERO_CROSS);

	SET_BIT(PORT_TRIAC1,BIT_TRIAC1,1);
	LPC_TMR16B1->MR0=power;
	LPC_TMR16B1->TCR=2;
	LPC_TMR16B1->TCR=1;
}

void TIMER16_1_IRQHandler(void)
{
	if(LPC_TMR16B1->MR0<10000)
	{
		SET_BIT(PORT_TRIAC1,BIT_TRIAC1,0);
		LPC_TMR16B1->MR0=10000;
	}
	else if(LPC_TMR16B1->MR0==10000)//seconde zero crossing
	{
		SET_BIT(PORT_TRIAC1,BIT_TRIAC1,1);
		LPC_TMR16B1->MR0=10000+power;
	}
	else
	{
		SET_BIT(PORT_TRIAC1,BIT_TRIAC1,0);
	}
	LPC_TMR16B1->IR = 0xFF;
}
