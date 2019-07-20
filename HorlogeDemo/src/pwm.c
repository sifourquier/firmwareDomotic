#include "timer32.h"
#include "LPC11xx.h"
#include "pwm.h"
#include "timer16.h"

volatile uint64_t time=0;
volatile uint32_t timeS=0;
volatile uint8_t flagNewSec=0;
volatile uint8_t flagNew1ms=0;

#define PORT_PWM1 1
#define BIT_PWM1 1

#define PORT_PWM2 1
#define BIT_PWM2 2

#define PORT_PWM3 0
#define BIT_PWM3 11

#define SET_BIT(PORT,BIT,x) ((LPC_GPIO_TypeDef*)(LPC_GPIO_BASE+0x10000*PORT))->MASKED_ACCESS[1<<BIT]=((x ? 1 : 0)<<BIT);


static const uint8_t pwmOrder[3]={0,1,2};

void pwmInit()
{
	init_timer32(1, DIV_FREQU_PWM*MAX_POWER,1);//1k Hz
	enable_timer32(1);

	LPC_TMR32B1->MR0=0;
	LPC_TMR32B1->MR1=0;
}

void setPwm(uint8_t numPwm,uint16_t val)
{
	numPwm=pwmOrder[numPwm];
	if(val>999)
		val=999;
	if(numPwm==2)
	{
		if(val>955)
			val=955;	//limite max pwm for software pwm else bug
		if(val<30 && val!=0)
			val=30;	//limite max pwm for software pwm else bug
	}
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
	static uint32_t oldTimerS=0;
	static uint32_t oldTimerMs=0;
	uint32_t timeMs;
	if(LPC_TMR32B1->IR&(1<<3))
	{
		SET_BIT(PORT_PWM3,BIT_PWM3,0);
		LPC_TMR32B1->IR = (1<<3);
		time+=DIV_FREQU_PWM*MAX_POWER/MHZ_PRESCALE;
		timeMs=time/1000;
		if(timeMs>oldTimerMs)
		{
			flagNew1ms=1;
			timeS=timeMs/1000;
			if(timeS>oldTimerS)
			{
				flagNewSec=1;
				oldTimerS=timeS;
			}
			oldTimerMs=timeMs;
		}


	}
	if(LPC_TMR32B1->IR&(1<<2))
	{
		if(LPC_TMR32B1->MR[2]<(MAX_POWER-15)*DIV_FREQU_PWM)
			SET_BIT(PORT_PWM3,BIT_PWM3,1);
		LPC_TMR32B1->IR = (1<<2);
	}

}
