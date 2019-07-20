#include "timer32.h"
#include "LPC11xx.h"
#include "board.h"
#include "pwm.h"
#include "timer16.h"

volatile uint64_t time=0;
volatile uint64_t timeS=0;
volatile uint8_t flagNewSec=0;

static const uint8_t pwmOrder[3]={PWM_ORDER};

void pwmInit()
{
	init_timer32(1, DIV_FREQU_PWM*MAX_POWER);//480 Hz
	enable_timer32(1);
	LPC_TMR32B1->MR0=0;
	LPC_TMR32B1->MR1=0;
}

void setPwm(uint8_t numPwm,uint16_t val)
{
	numPwm=pwmOrder[numPwm];
	if(val>999)
		val=999;
	if(val==0)
	{
		LPC_TMR32B1->PWMC&=~(1<<numPwm); //out = 0
		LPC_TMR32B1->EMR&=~(1<<numPwm);
	}
	else
		LPC_TMR32B1->PWMC|=(1<<numPwm); //out = PWM
	if(numPwm<3)
	{
		LPC_TMR32B1->MR[numPwm]=(MAX_POWER-val-1)*DIV_FREQU_PWM;
	}
}

void TIMER32_1_IRQHandler(void)
{
	time+=DIV_FREQU_PWM*MAX_POWER/MHZ_PRESCALE;
	if(time/1000000>timeS)
		flagNewSec=1;
	timeS=time/1000000;

	if(LPC_TMR32B1->IR&(1<<3))
	{
		SET_BIT(PORT_PWM3,BIT_PWM3,0);
		LPC_TMR32B1->IR = (1<<3);
	}
	if(LPC_TMR32B1->IR&(1<<2))
	{
		if(LPC_TMR32B1->MR[2]<(MAX_POWER-15)*DIV_FREQU_PWM)
			SET_BIT(PORT_PWM3,BIT_PWM3,1);
		LPC_TMR32B1->IR = (1<<2);
	}

}
