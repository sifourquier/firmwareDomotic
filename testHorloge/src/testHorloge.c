/*
===============================================================================
 Name        : testHorloge.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include "ssp.h"
#include "charSet.h"
#include "screen.h"

#include <cr_section_macros.h>

#include <stdio.h>

#define abs(x) ((x)<0 ? -(x) : (x))

int8_t screen[WIDTH][HEIGHT]={0};
// TODO: insert other include files here

// TODO: insert other definitions and declarations here

void delay(int t)
{
	volatile int l,l2;
	for(l=0;l<t;l++)
		for(l2=0;l2<3000;l2++);
}

void updateScreen(int8_t screen[WIDTH][HEIGHT])
{
	const int8_t screendir[WIDTH][HEIGHT]={
			{0,0,1,0,0,0},
			{0,0,1,1,0,0}};
	/*const uint8_t screenpower[WIDTH][HEIGHT]={{7,9,13,14,14,14}};
	const int8_t screenoffset[WIDTH][HEIGHT]={{0,-2,-4,-7,-7,-7}};//pwm value for 0 mangetic field (electromanget=-aiment)*/

	//const uint8_t screenpower[WIDTH][HEIGHT]={{6,6,6,6,6,6}};
	//const int8_t screenoffset[WIDTH][HEIGHT]={{-3,-3,-3,-3,-3,-3}};
	uint8_t buffer[WIDTH*HEIGHT/2]; //3bit bobine ssp en 6bit
	uint32_t pt=0;
	for(int x=0;x<WIDTH;x++)
	{
		for(int y=0;y<HEIGHT;y++)
		{
			int temp;
			temp=screen[x][y];
			//temp=temp*screenpower[x][y]/6+screenoffset[x][y];
			temp-=3;
			if(temp>3)
				temp=3;
			if(temp<-3)
				temp=-3;


			if((temp<0)==screendir[x][y])
			{
				temp=(abs(temp));
			}
			else
			{
				temp=(abs(temp))|(1<<2);
			}
			if(pt%2==0)
			{
				buffer[pt/2]=temp<<3;
			}
			else
			{
				buffer[pt/2]|=temp;
			}
			pt++;
		}
	}
	SSPSend(0, buffer, sizeof(buffer));
}

void updateScreenSmooth(int8_t screen[WIDTH][HEIGHT],int time)
{
	static int8_t currentScreen[WIDTH][HEIGHT]={0};
	int equal=0;
	while(!equal)
	{
		equal=1;
		for(int x=0;x<WIDTH;x++)
		{
			for(int y=0;y<HEIGHT;y++)
			{
				if(currentScreen[x][y]<screen[x][y])
				{
					currentScreen[x][y]+=2;
					if(currentScreen[x][y]>screen[x][y])
						currentScreen[x][y]=screen[x][y];
					equal=0;
				}
				else if(currentScreen[x][y]>screen[x][y])
				{
					currentScreen[x][y]-=1;
					if(currentScreen[x][y]<screen[x][y])
						currentScreen[x][y]=screen[x][y];
					equal=0;
				}
			}
		}
		updateScreen(currentScreen);
		delay(50);//for protocol
		delay(time);

	}
}

void initClockOut() //20mhz clock out at MAT2[3] (max freque for PIC16F5X at 3.3V)
{
	LPC_SC->PCLKSEL1|=1UL<<12;
	LPC_SC->PCONP|=1<<22;//power timer 2
	LPC_TIM2->TCR = 2;//reset
	LPC_TIM2->MR3 = 2;
	LPC_TIM2->MCR = 2<<9;				/*Reset on MR3 */
	LPC_TIM2->EMR = 3<<10;//toggle MR3 out
	LPC_PINCON->PINSEL0 &= ~(0x3UL<<18);
	LPC_PINCON->PINSEL0 |= (0x3UL<<18);//P0_9 -> MATCH 2.3
	LPC_TIM2->TCR = 1;//run
}

int main(void) {
	printf("Hello World\n");
	initClockOut();
	SSP0Init();

#define TSHORT 2000
#define TLONG 2000

#define MAX_POWER 6
#define HIGH_POWER 14
#define LOW_POWER 5



	/*screen[0][0]=3;
	screen[0][1]=2;
	screen[0][2]=1;
	screen[0][3]=1;
	screen[0][4]=0;
	screen[0][5]=0;
	updateScreen(screen);
	delay(2000);*/

	/*while(1)
	{
		for(int l=0;l<7;l++)
		{
			screen[0][0]=l;
			screen[0][1]=l;
			screen[0][2]=l;
			screen[0][3]=l;
			screen[0][4]=l;
			screen[0][5]=l;
			updateScreen(screen);
		}
	}*/

	{

		while(1)
		{

			screen[0][0]=MAX_POWER;
			screen[0][1]=MAX_POWER-1;
			screen[0][2]=MAX_POWER-1;
			screen[0][3]=MAX_POWER;
			screen[0][4]=MAX_POWER;
			screen[0][5]=MAX_POWER;
			screen[1][0]=MAX_POWER;
			screen[1][1]=MAX_POWER-1;
			screen[1][2]=MAX_POWER-1;
			screen[1][3]=MAX_POWER;
			screen[1][4]=MAX_POWER;
			screen[1][5]=MAX_POWER;
			updateScreenSmooth(TSHORT);
			delay(30000);

			/*screen[0][0]=0;
			screen[0][1]=MAX_POWER/2;
			screen[0][2]=MAX_POWER/2;
			screen[0][3]=MAX_POWER/2+1;
			screen[0][4]=MAX_POWER/2+1;
			screen[0][5]=MAX_POWER/2+1;
			screen[1][0]=MAX_POWER/2;
			screen[1][1]=MAX_POWER/2;
			screen[1][2]=2;
			screen[1][3]=2;
			screen[1][4]=2;
			screen[1][5]=MAX_POWER/2+1;
			updateScreenSmooth(TSHORT);*/

			screen[0][0]=MAX_POWER/2;
			screen[0][1]=MAX_POWER/2;
			screen[0][2]=MAX_POWER/2+1;
			screen[0][3]=MAX_POWER/2+1;
			screen[0][4]=MAX_POWER/2+2;
			screen[0][5]=MAX_POWER/2+3;
			screen[1][0]=MAX_POWER/2;
			screen[1][1]=MAX_POWER/2;
			screen[1][2]=2;
			screen[1][3]=2;
			screen[1][4]=2;
			screen[1][5]=MAX_POWER/2+3;
			updateScreenSmooth(TSHORT);

			delay(50000);

			screen[0][0]=MAX_POWER;
			screen[0][1]=MAX_POWER;
			screen[0][2]=MAX_POWER;
			screen[0][3]=MAX_POWER;
			screen[0][4]=MAX_POWER;
			screen[0][5]=2;
			for(int l=0;l<6;l++)
			{
				screen[1][l]=screen[0][l];
			}
			updateScreenSmooth(TSHORT);

			screen[0][0]=MAX_POWER;
			screen[0][1]=MAX_POWER;
			screen[0][2]=MAX_POWER;
			screen[0][3]=MAX_POWER;
			screen[0][4]=2;
			screen[0][5]=2;
			for(int l=0;l<6;l++)
			{
				screen[1][l]=screen[0][l];
			}
			updateScreenSmooth(TSHORT);

			screen[0][0]=MAX_POWER;
			screen[0][1]=MAX_POWER;
			screen[0][2]=MAX_POWER;
			screen[0][3]=2;
			screen[0][4]=2;
			screen[0][5]=2;
			for(int l=0;l<6;l++)
			{
				screen[1][l]=screen[0][l];
			}
			updateScreenSmooth(TSHORT);

			screen[0][0]=MAX_POWER;
			screen[0][1]=MAX_POWER;
			screen[0][2]=2;
			screen[0][3]=2;
			screen[0][4]=2;
			screen[0][5]=2;
			for(int l=0;l<6;l++)
			{
				screen[1][l]=screen[0][l];
			}
			updateScreenSmooth(TSHORT);

			screen[0][0]=MAX_POWER;
			screen[0][1]=2;
			screen[0][2]=2;
			screen[0][3]=2;
			screen[0][4]=2;
			screen[0][5]=2;
			for(int l=0;l<6;l++)
			{
				screen[1][l]=screen[0][l];
			}
			updateScreenSmooth(TSHORT);

			screen[0][0]=MAX_POWER/2;
			screen[0][1]=2;
			screen[0][2]=2;
			screen[0][3]=2;
			screen[0][4]=2;
			screen[0][5]=2;
			for(int l=0;l<6;l++)
			{
				screen[1][l]=screen[0][l];
			}
			updateScreenSmooth(TSHORT);
			delay(20000);
		}

		screen[0][0]=LOW_POWER;
		screen[0][1]=HIGH_POWER;
		screen[0][2]=0;
		screen[0][3]=0;
		screen[0][4]=0;
		screen[0][5]=0;
		updateScreenSmooth(TLONG);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=HIGH_POWER;
		screen[0][2]=0;
		screen[0][3]=0;
		screen[0][4]=0;
		screen[0][5]=0;
		updateScreenSmooth(TSHORT);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=LOW_POWER;
		screen[0][2]=HIGH_POWER;
		screen[0][3]=0;
		screen[0][4]=0;
		screen[0][5]=0;
		updateScreenSmooth(TLONG);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=0;
		screen[0][2]=HIGH_POWER;
		screen[0][3]=0;
		screen[0][4]=0;
		screen[0][5]=0;
		updateScreenSmooth(TSHORT);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=0;
		screen[0][2]=LOW_POWER-1;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=0;
		screen[0][5]=0;
		updateScreenSmooth(TLONG);

		screen[0][0]=HIGH_POWER;
		screen[0][1]=0;
		screen[0][2]=0;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=0;
		screen[0][5]=0;
		updateScreenSmooth(TSHORT);

		screen[0][0]=LOW_POWER;
		screen[0][1]=HIGH_POWER;
		screen[0][2]=0;
		screen[0][3]=LOW_POWER;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=0;
		updateScreenSmooth(TLONG);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=HIGH_POWER;
		screen[0][2]=0;
		screen[0][3]=0;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=0;
		updateScreenSmooth(TSHORT);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=LOW_POWER;
		screen[0][2]=HIGH_POWER;
		screen[0][3]=0;
		screen[0][4]=LOW_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TLONG);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=0;
		screen[0][2]=HIGH_POWER;
		screen[0][3]=0;
		screen[0][4]=0;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TSHORT);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=0;
		screen[0][2]=LOW_POWER-1;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=0;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TLONG);

		screen[0][0]=HIGH_POWER;
		screen[0][1]=0;
		screen[0][2]=0;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=0;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TSHORT);

		screen[0][0]=LOW_POWER;
		screen[0][1]=HIGH_POWER;
		screen[0][2]=0;
		screen[0][3]=LOW_POWER;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TLONG);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=HIGH_POWER;
		screen[0][2]=0;
		screen[0][3]=0;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TSHORT);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=LOW_POWER;
		screen[0][2]=HIGH_POWER;
		screen[0][3]=0;
		screen[0][4]=LOW_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TLONG);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=0;
		screen[0][2]=HIGH_POWER;
		screen[0][3]=0;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TSHORT);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=0;
		screen[0][2]=LOW_POWER-1;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TLONG);

		screen[0][0]=HIGH_POWER;
		screen[0][1]=0;
		screen[0][2]=0;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TSHORT);

		screen[0][0]=LOW_POWER;
		screen[0][1]=HIGH_POWER;
		screen[0][2]=0;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TLONG);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=HIGH_POWER;
		screen[0][2]=0;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TSHORT);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=0;
		screen[0][2]=MAX_POWER;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TLONG);

		screen[0][0]=HIGH_POWER;
		screen[0][1]=0;
		screen[0][2]=MAX_POWER/2;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(TSHORT);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=MAX_POWER/2;
		screen[0][2]=HIGH_POWER;
		screen[0][3]=HIGH_POWER;
		screen[0][4]=HIGH_POWER;
		screen[0][5]=HIGH_POWER;
		updateScreenSmooth(0);

		screen[0][0]=MAX_POWER/2;
		screen[0][1]=MAX_POWER;
		screen[0][2]=MAX_POWER;
		screen[0][3]=MAX_POWER;
		screen[0][4]=MAX_POWER;
		screen[0][5]=MAX_POWER;
		updateScreenSmooth(TLONG);
		delay(100000);
	}

	return 0 ;
}
