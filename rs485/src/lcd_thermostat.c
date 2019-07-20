/*
 * lcd_thermostat.c
 *
 *  Created on: 23 déc. 2018
 *      Author: simon
 *
 *      lcd controleur are HT1621B
 */
#include "lcd_thermostat.h"

#define CS SET_BIT(PORT_nCS_LCD,BIT_nCS_LCD,0);
#define nCS SET_BIT(PORT_nCS_LCD,BIT_nCS_LCD,1);
#define COMMON 2//p8 datasheet
#define BIAS 0
#define CPOLCPHA 0b11

#define TIME_CS for(volatile uint32_t l=0;l<100000;l++);

//power measurement
//12V in vcc board =5V internal linear regulator 20mA 12V
//12V in vcc board =5V internal linear switching 17mA 12V
//12V in vcc board 4.45V internal linear switching 15mA 12V

#ifdef LCD_CHEAP_2_TEMP_5_PLUS_2_DAY_4_HOME_IN_OUT_SUN_MOON_SNOW_LOCK_HAND_TIME
#define S_B (1<<6) //segment Temperature humidity
#define S_G (1<<5)
#define S_C (1<<4)
#define S_A (1<<3)
#define S_F (1<<2)
#define S_E (1<<1)
#define S_D (1<<0)

#define SR_A (1<<0)
#define SR_F (1<<1)
#define SR_E (1<<2)
#define SR_D (1<<3)
#define SR_B (1<<4)
#define SR_G (1<<5)
#define SR_C (1<<6)


#define SR_5_S3 ((1<<3)||(1<<5))
#define SR_5_S3 ((1<<3)||(1<<5))

#define SR_5_S3 ((1<<3)||(1<<5))
#define SR_5_S3 ((1<<3)||(1<<5))

uint8_t numberTo7segPart1[10]={
		S_A|S_D|S_E|S_F,//0
		0,//1
		S_A|S_E|S_D,//2
		S_A|S_D,//3
		S_F,//4
		S_A|S_F|S_D,//5
		S_A|S_D|S_E|S_F,//6
		S_A,//7
		S_A|S_D|S_E|S_F,//8
		S_A|S_D|S_F,//9
};

uint8_t numberTo7segPart2[10]={
		S_B|S_C,//0
		S_B|S_C,//1
		S_B|S_G,//2
		S_B|S_G|S_C,//3
		S_G|S_B|S_C,//4
		S_G|S_C,//5
		S_C|S_G,//6
		S_B|S_C,//7
		S_B|S_C|S_G,//8
		S_B|S_C|S_G,//9
};

uint8_t numberReverseTo7seg[10]={
		SR_A|SR_B|SR_C|SR_D|SR_E|SR_F,//0
		SR_B|SR_C,//1
		SR_A|SR_B|SR_G|SR_E|SR_D,//2
		SR_A|SR_B|SR_G|SR_C|SR_D,//3
		SR_F|SR_G|SR_B|SR_C,//4
		SR_A|SR_F|SR_G|SR_C|SR_D,//5
		SR_A|SR_C|SR_D|SR_E|SR_F|SR_G,//6
		SR_A|SR_B|SR_C,//7
		SR_A|SR_B|SR_C|SR_D|SR_E|SR_F|SR_G,//8
		SR_A|SR_B|SR_C|SR_D|SR_F|SR_G,//9
};

uint8_t lcdSet(t_lcd lcd) //need be called 13 time for fulle send
{
	static int cpu_split=0;

	static uint8_t data[15];
	if(cpu_split==0)
	{
		memset(data,0,sizeof(data));
		uint16_t write=0b101000000;
		uint8_t add=0;
		write|=add;
		data[2]|=(lcd.day1to7&1)<<2|(lcd.day1to7&2)|(lcd.day1to7&4)>>2;
		data[3]|=(lcd.day1to7&0x7C)<<1;
		data[14]|=lcd.in<<4;
		data[14]|=lcd.set<<5;

		if(lcd.big_digit<=999)//<0 = off
		{
			if(lcd.big_digit<0)
			{
				if(lcd.big_digit<-99)
					lcd.big_digit=-99;
				data[12]|=S_G;
				data[11]|=(numberReverseTo7seg[(-lcd.big_digit/10)%10]);
				if(lcd.big_digit_hide_decimal==0)
				{
					if((-lcd.big_digit)%10<5)
					{
						data[2]|=1<<3|1<<7;//AFDC + BE
					}
					else
					{
						data[2]|=1<<3;//AFDC
						data[12]|=1<<7;//G
					}
				}
			}
			else
			{
				if(lcd.big_digit>=100)
					data[12]|=(numberReverseTo7seg[(lcd.big_digit/100)%10]);
				data[11]|=(numberReverseTo7seg[(lcd.big_digit/10)%10]);
				if(lcd.big_digit_hide_decimal==0)
				{
					if((lcd.big_digit)%10<5)
					{
						data[2]|=1<<3|1<<7;//AFDC + BE
					}
					else
					{
						data[2]|=1<<3;//AFDC
						data[12]|=1<<7;//G
					}
				}
			}
		}

		data[11]|=(lcd.deg_c<<7);
		data[10]|=(lcd.deg_c2<<3);
		data[0]|=(lcd.twoPoints<<7);

		if(lcd.small_digit<=999)//>999 no printable
		{
			if(lcd.small_digit<0) //-9<x<0 print -x
			{
				if(lcd.small_digit<-99)//x<-9 porint -
				{
					data[10]|=S_G; //draw -
				}
				else
				{
					data[9]|=S_G; //draw -
					data[9]|=numberTo7segPart1[(-lcd.small_digit/10)%10];
					data[10]|=numberTo7segPart2[(-lcd.small_digit/10)%10];
					if(lcd.small_digit_hide_decimal==0)
					{
						if((-lcd.small_digit)%10<5)
						{
							data[9]|=1<<7;//AFDC + BE
							data[10]|=1<<0;
						}
						else
						{
							data[9]|=1<<7;//AFDC + BE
							data[10]|=1<<7;//G
						}
					}
				}
			}
			else
			{
				if(lcd.small_digit>=100)
				{
					data[3]|=numberTo7segPart1[(lcd.small_digit/100)%10];
					data[9]|=numberTo7segPart2[(lcd.small_digit/100)%10];
				}
				data[9]|=numberTo7segPart1[(lcd.small_digit/10)%10];
				data[10]|=numberTo7segPart2[(lcd.small_digit/10)%10];
				if(lcd.small_digit_hide_decimal==0)
				{
					if((lcd.small_digit)%10<5)
					{
						data[9]|=1<<7;//AFDC + BE
						data[10]|=1<<0;
					}
					else
					{
						data[9]|=1<<7;//AFDC + BE
						data[10]|=1<<7;//G
					}
				}
			}
		}

		if(lcd.small_digit_hour[0]<=29)//>99 no printable
		{
			if(lcd.small_digit_hour[0]>=0) //<0 no printable
			{
				if((lcd.small_digit_hour[0])/10==1)
				{
					data[1]|=1<<7; //B
					data[14]|=1<<7; //C
				}
				else if((lcd.small_digit_hour[0])/10==2)
				{
					data[1]|=1<<7; //B
					data[14]|=1<<6; //C
				}
				data[14]|=numberTo7segPart1[(lcd.small_digit_hour[0])%10];
				data[0]|=numberTo7segPart2[(lcd.small_digit_hour[0])%10];
			}
		}

		if(lcd.small_digit_hour[1]<=99)//>99 no printable
		{
			if(lcd.small_digit_hour[1]<0)
			{
				if(lcd.small_digit_hour[1]>=-9)
				{
					data[1]|=S_G; //G
					data[1]|=numberTo7segPart1[(lcd.small_digit_hour[1])%10];
					data[2]|=numberTo7segPart2[(lcd.small_digit_hour[1])%10];
				}
			}
			else
			{
				data[0]|=numberTo7segPart1[(lcd.small_digit_hour[1]/10)];
				data[1]|=numberTo7segPart2[(lcd.small_digit_hour[1]/10)];
				data[1]|=numberTo7segPart1[(lcd.small_digit_hour[1])%10];
				data[2]|=numberTo7segPart2[(lcd.small_digit_hour[1])%10];
			}
		}

		data[10]|=((lcd.home>>2)&0x01)<<1|((lcd.home>>3)&0x01)<<2;

		data[13]|=(lcd.key<<0)|(lcd.snow<<1)|(lcd.hand<<2)|(lcd.heat<<3)|(lcd.moon<<4)|(lcd.sun<<5);
		data[13]|=((lcd.home>>0)&0x01)<<6|((lcd.home>>1)&0x01)<<7;
		data[14]|=(lcd.in<<4)|(lcd.in<<5);

		CS;
		SSP_Init(0,150000,9,CPOLCPHA);
		SSP_Send_16bit_nowait_end(0, write);
	}
	else if((cpu_split>0) && (cpu_split<sizeof(data)+1))
	{
		SSP_wait_end(0);//wait end of preview send
		if(cpu_split==1)
			SSP_Init(0,150000,8,CPOLCPHA);
		SSP_Send_16bit_nowait_end(0, data[cpu_split-1]);

	}
	else
	{
		SSP_wait_end(0);//wait end of preview send
		cpu_split=-1;
		nCS;
	}
	cpu_split++;
	return cpu_split;
}

#else
#define S_E (1<<6) //segment Temperature humidity
#define S_G (1<<5)
#define S_F (1<<4)
#define S_D (1<<3)
#define S_C (1<<2)
#define S_B (1<<1)
#define S_A (1<<0)

#define SH_D (1<<7) //segment hour
#define SH_C (1<<6)
#define SH_B (1<<5)
#define SH_A (1<<4)
#define SH_E (1<<2)
#define SH_G (1<<1)
#define SH_F (1<<0)

uint8_t numberTo7seg[10]={
		S_A|S_B|S_C|S_D|S_E|S_F,//0
		S_B|S_C,//1
		S_A|S_B|S_G|S_E|S_D,//2
		S_A|S_B|S_G|S_C|S_D,//3
		S_F|S_G|S_B|S_C,//4
		S_A|S_F|S_G|S_C|S_D,//5
		S_A|S_C|S_D|S_E|S_F|S_G,//6
		S_A|S_B|S_C,//7
		S_A|S_B|S_C|S_D|S_E|S_F|S_G,//8
		S_A|S_B|S_C|S_D|S_F|S_G,//9
};

uint8_t numberTo7segFirst[10]={
		0,//
		S_B|S_C,//1
		S_A|S_B|S_G|S_E|S_D,//2
		S_A|S_B|S_G|S_C|S_D,//3
		S_F|S_G|S_B|S_C,//4
		S_A|S_F|S_G|S_C|S_D,//5
		S_A|S_C|S_D|S_E|S_F|S_G,//6
		S_A|S_B|S_C,//7
		S_A|S_B|S_C|S_D|S_E|S_F|S_G,//8
		S_A|S_B|S_C|S_D|S_F|S_G,//9
};

uint8_t numberTo7segHour[10]={
		SH_A|SH_B|SH_C|SH_D|SH_E|SH_F,//0
		SH_B|SH_C,//1
		SH_A|SH_B|SH_G|SH_E|SH_D,//2
		SH_A|SH_B|SH_G|SH_C|SH_D,//3
		SH_F|SH_G|SH_B|SH_C,//4
		SH_A|SH_F|SH_G|SH_C|SH_D,//5
		SH_A|SH_C|SH_D|SH_E|SH_F|SH_G,//6
		SH_A|SH_B|SH_C,//7
		SH_A|SH_B|SH_C|SH_D|SH_E|SH_F|SH_G,//8
		SH_A|SH_B|SH_C|SH_D|SH_F|SH_G,//9
};

uint8_t numberTo7segHourFirst[10]={
		0,//
		SH_B|SH_C,//1
		SH_A|SH_B|SH_G|SH_E|SH_D,//2
		SH_A|SH_B|SH_G|SH_C|SH_D,//3
		SH_F|SH_G|SH_B|SH_C,//4
		SH_A|SH_F|SH_G|SH_C|SH_D,//5
		SH_A|SH_C|SH_D|SH_E|SH_F|SH_G,//6
		SH_A|SH_B|SH_C,//7
		SH_A|SH_B|SH_C|SH_D|SH_E|SH_F|SH_G,//8
		SH_A|SH_B|SH_C|SH_D|SH_F|SH_G,//9
};

uint8_t lcdSet(t_lcd lcd) //need be called 13 time for fulle send
{
	static int cpu_split=0;

	static uint8_t data[11];
	if(cpu_split==0)
	{
		uint16_t write=0b101000000;
		uint8_t add=0;
		write|=add;
		data[0]=lcd.week<<7|(lcd.day1to7);
		data[1]=lcd.thermometer<<7;
		data[2]=lcd.set<<7;
		if(lcd.big_digit<=999)//<0 = off
		{
			if(lcd.big_digit<0)
			{
				if(lcd.big_digit<-99)
					lcd.big_digit=-99;
				data[1]=S_G;
				data[2]|=(numberTo7seg[(-lcd.big_digit/10)%10]);
				data[3]=(numberTo7seg[(-lcd.big_digit)%10]);
			}
			else
			{
				data[1]|=(numberTo7segFirst[(lcd.big_digit/100)%10]);
				data[2]|=(numberTo7seg[(lcd.big_digit/10)%10]);
				data[3]=(numberTo7seg[(lcd.big_digit)%10]);
			}
		}
		data[4]=(lcd.home<<4)|(lcd.deg_c<<3)|(lcd.deg_c2<<2)|(lcd.rh<<1)|(lcd.point<<0);
		data[5]=(lcd.h<<3);
		data[6]=(lcd.twoPoints<<3);
		if(lcd.small_digit_hour[1]<=99)//>99 no printable
		{
			if(lcd.small_digit_hour[1]<0) //-9<x<0 print -x
			{
				if(lcd.small_digit_hour[1]<-9)//x<-9 porint -
				{
					data[5]|=SH_G;
				}
				else
				{
					data[6]|=SH_G;
					data[5]|=numberTo7segHour[(-lcd.small_digit_hour[1])%10];
				}
			}
			else
			{
				data[5]|=numberTo7segHour[(lcd.small_digit_hour[1])%10];
				data[6]|=numberTo7segHour[(lcd.small_digit_hour[1]/10)%10];
			}
		}
		data[7]=(lcd.on<<3);
		data[8]=(lcd.off<<3);
		if(lcd.small_digit_hour[0]<=99)//>99 no printable
		{
			if(lcd.small_digit_hour[0]<0) //-9<x<0 print -x
			{
				if(lcd.small_digit_hour[0]<-9)//x<-9 porint -
				{
					data[7]|=SH_G;
				}
				else
				{
					data[8]|=SH_G;
					data[7]|=numberTo7segHour[(-lcd.small_digit_hour[0])%10];
				}
			}
			else
			{
				data[7]|=numberTo7segHour[(lcd.small_digit_hour[0])%10];
				data[8]|=numberTo7segHour[(lcd.small_digit_hour[0]/10)%10];
			}
		}
		data[9]=(lcd.fan<<5)|(lcd._auto<<4)|(lcd.heat<<3)|(lcd.unknow<<2)|(lcd.pm<<1)|(lcd.am<<0);
		data[10]=(lcd.snow<<7)|(lcd.sun<<6)|(lcd.fan_unknow<<5)|(lcd.moon<<4)|(lcd.leaf<<3)|(lcd.wifi<<2)|(lcd.hand<<1)|(lcd.key<<0);
		//data[0]=cpt;
		CS;
		SSP_Init(0,150000,9,CPOLCPHA);
		SSP_Send_16bit_nowait_end(0, write);
	}
	else if((cpu_split>0) && (cpu_split<sizeof(data)+1))
	{
		SSP_wait_end(0);//wait end of preview send
		if(cpu_split==1)
			SSP_Init(0,150000,8,CPOLCPHA);
		SSP_Send_16bit_nowait_end(0, data[cpu_split-1]);

	}
	else
	{
		SSP_wait_end(0);//wait end of preview send
		cpu_split=-1;
		nCS;
	}
	cpu_split++;
	return cpu_split;
}

#endif

void lcdInit()
{
	uint8_t sysEn[]={0b100000,0b000010};//on
	uint8_t lcdOn[]={0b100000,0b000110};//LCD on
	uint8_t commonBias[]={0b100001,0b000000|(COMMON<<3)|(BIAS<1)};

	//LPC_IOCON->PIO0_8=0|DIGITAL|OPENDRAIN;//use MISO as CS
	LPC_IOCON->PIO0_9=1|DIGITAL|OPENDRAIN;/*MOSI*/\
	LPC_IOCON->PIO0_6=2|DIGITAL|OPENDRAIN;
	//SET_OUT(PORT_MISO,BIT_MISO);
	nCS;
	SSP_Init(0,150000,6,CPOLCPHA);//ssp0 150khz 12bit
	CS;
	SSP_Send(0, sysEn, 2);
	nCS;
	TIME_CS;
	CS;
	SSP_Send(0, lcdOn, 2);
	nCS;
	TIME_CS;
	CS;
	SSP_Send(0, commonBias, 3);
	nCS;
	TIME_CS;
}

uint8_t lcdMap() //enable segment by segment for help to map LCD
{
	while(1)
	{
		static int cpt=0;
		static uint8_t data[16];
		for(int l=0;l<sizeof(data);l++)
		{
			data[l]=0xFF;
		}
		data[cpt/8]=~(1<<(cpt&0x7));
		cpt++;


		CS;
		SSP_Init(0,150000,9,CPOLCPHA);
		uint16_t write=0b101000000;
		SSP_Send_16bit_nowait_end(0, write);
		SSP_wait_end(0);//wait end of preview send
		SSP_Init(0,150000,8,CPOLCPHA);
		for(volatile int l=0;l<sizeof(data);l++)
		{
			SSP_Send_16bit_nowait_end(0, data[l]);
			SSP_wait_end(0);//wait end of preview send

		}
		nCS;
	}
}


