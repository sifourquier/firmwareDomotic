/*
===============================================================================
 Name        : HorlogeDemo.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "LPC11xx.h"

#include <cr_section_macros.h>

#include <stdio.h>
#include "timer16.h"
#include "timer32.h"
#include "pwm.h"
#include "screenFeroFluid.h"

#define GET_BIT(PORT,BIT) ((((LPC_GPIO_TypeDef*)(LPC_GPIO_BASE+0x10000*PORT))->DATA>>BIT)&1)
#define SET_BIT(PORT,BIT,x) ((LPC_GPIO_TypeDef*)(LPC_GPIO_BASE+0x10000*PORT))->MASKED_ACCESS[1<<BIT]=((x ? 1 : 0)<<BIT);
#define SET_OUT(PORT,BIT) ((LPC_GPIO_TypeDef*)(LPC_GPIO_BASE+0x10000*PORT))->DIR|=1<<BIT;
#define SET_IN(PORT,BIT) ((LPC_GPIO_TypeDef*)(LPC_GPIO_BASE+0x10000*PORT))->DIR&=~(1<<BIT);

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

void configClock()
{
	LPC_SYSCON->PDRUNCFG     &= ~(1 << 5);          /* Power-up System Osc      */
	LPC_SYSCON->SYSOSCCTRL    = 0;
	for (volatile int i = 0; i < 200; i++) __NOP();

	LPC_SYSCON->SYSPLLCLKSEL  = 1;   /* Select PLL Input = IRC*/
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;               /* Update Clock Source      */
	LPC_SYSCON->SYSPLLCLKUEN  = 0x00;               /* Toggle Update Register   */
	LPC_SYSCON->SYSPLLCLKUEN  = 0x01;
	while (!(LPC_SYSCON->SYSPLLCLKUEN & 0x01));     /* Wait Until Updated       */
	LPC_SYSCON->SYSPLLCTRL    = 0x03; //P=(2^0) PLL M=(3+1) M=1 > 7mA / M=2 > 10mA M=4/11mA
	LPC_SYSCON->PDRUNCFG     &= ~(1 << 7);          /* Power-up SYSPLL          */
	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));           /* Wait Until PLL Locked    */

	while (!(LPC_SYSCON->SYSPLLSTAT & 0x01));
	LPC_SYSCON->MAINCLKSEL    = 0x03;     			/* Select PLL Clock for main Clock  */
	LPC_SYSCON->MAINCLKUEN    = 0x01;               /* Update MCLK Clock Source */
	LPC_SYSCON->MAINCLKUEN    = 0x00;               /* Toggle Update Register   */
	LPC_SYSCON->MAINCLKUEN    = 0x01;
	while (!(LPC_SYSCON->MAINCLKUEN & 0x01));       /* Wait Until Updated       */

	LPC_SYSCON->SYSAHBCLKDIV  = 1; //clock divider =1

}

int main(void) {
	int s=0;
	int h=0;
	int m=0;
	int sw,oldSw;

	configClock();

	LPC_SYSCON->SYSAHBCLKCTRL |= (0x1UL << 16); //power IOCON
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6); //power gpio

	LPC_IOCON->SCK_LOC=1;/*SCK0 on PIO0_6*/
	LPC_IOCON->PIO2_11=1|DIGITAL;/*SCK0 differente of rsr485 board*/
	LPC_IOCON->PIO0_8=1|DIGITAL;/*MISO0*/
	LPC_IOCON->PIO0_9=1|DIGITAL;/*MOSI0*/

	SET_OUT(2,10);
	SET_BIT(2,10,0);
	SET_OUT(2,7);
	SET_BIT(2,7,0);
	SET_OUT(3,5);
	SET_BIT(3,5,0);
	SET_OUT(1,11);
	SET_BIT(1,11,0);

	pwmInit();
	screenInit();
	screenBare();

    // Force the counter to be placed into memory
    volatile static int i = 0 ;
    // Enter an infinite loop, just incrementing a counter
    while(1) {
    	if(flagNew1ms)
		{
			screenComputeMs();
			flagNew1ms--;
		}
		if(flagNewSec)
		{

			s++;
			flagNewSec=0;
			if(s>=60)
			{
				s=0;
				m++;
			}
			if(m>=60)
			{
				m=0;
				h++;
			}
			if(h>=24)
			{
				h=0;
			}
			printTime(h, m, s);
		}
		sw=GET_BIT(3,3)|(GET_BIT(2,9)<<1)|(GET_BIT(2,6)<<2)|(GET_BIT(3,4)<<3)|(GET_BIT(1,10)<<4);
		if(oldSw!=sw)
		{
			if(!(sw&(1<<0)))
			{
				h++;
				if(h>23)
					h=0;
			}
			if(!(sw&(1<<1)))
			{
				h--;
				if(h<0)
					h=23;
			}
			if(!(sw&(1<<2)))
			{
				m++;
				if(m>60)
					m=0;
			}
			if(!(sw&(1<<3)))
			{
				m--;
				if(m<0)
					m=60;
			}
			if(!(sw&(1<<4)))
			{
				s=0;
			}
			printTime(h, m, s);
		}
		oldSw=sw;
    }
    return 0 ;
}
