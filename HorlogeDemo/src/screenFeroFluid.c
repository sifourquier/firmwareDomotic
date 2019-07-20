/*
 * screen.c
 *
 *  Created on: 20 mars 2019
 *      Author: simon
 */
#include "screenFeroFluid.h"
#include "ssp.h"
#include "timer16.h"
#include "charSet.h"
#include "pwm.h"
#include "LPC11xx.h"

#define LIMITE_POWER (6000)

#define abs(x) ((x)<0 ? -(x) : (x))

static int8_t currentScreen[WIDTH][HEIGHT]={0};
static int8_t screen[WIDTH][HEIGHT];

static volatile int smooth=0;
static volatile int newData=1;
static int power=0;


static volatile int8_t screendir[WIDTH][HEIGHT]={
		{+1,+1,-1,-1, 1,-1},//2
		{+1,-1,+1,+1,-1,-1},//3
		{+1,-1,+1,+1,+1,-1},//4
		{+1,+1,+1,-1,+1,-1},//5--
		{+1,-1,+1,+1,-1,-1},//6
		{-1,+1,-1,-1,+1,+1},//7--
		{-1,-1,-1,-1,-1,+1},//8
		{-1,-1,-1,+1,-1,-1},//9--
		{+1,-1,-1,+1,-1,-1},//10
		{-1,+1,+1,-1,-1,-1},//11--
		{-1,-1,+1,+1,-1,+1},//12
		{-1,+1,+1,-1,-1,+1},//13--
		{+1,+1,-1,-1,-1,-1},//14
		{+1,-1,-1,-1,-1,+1},//15--
		{-1,+1,-1,-1,-1,+1},//16
		{+1,-1,-1,+1,+1,-1},//17
		{-1,-1,-1,-1,-1,+1}//18
};

void screenInit()
{
	memset(screen,0,sizeof(screen));
	for(int y=0;y<HEIGHT;y++)
	{
		screen[3][y]=-2;
		screen[4][y]=-2;
		screen[5][y]=-2;
	}
	SSP_Init(0,120000,6,0b00);//ssp0 1khz 6bit cpol=0 cpha=0
	LPC_SYSCON->SSP0CLKDIV=2*120;//div spi clk by 100
	//(freque pwm pic=20Khz -> freque loop = 80khz but when no data
	//i take 40khz with jitter for eache pic -> 40khz/18/2=1.1khz /2 for H en L level
	//1.1khz -> 1100/108/3 (108 manget 3bit) -> 3.4Hz

	LPC_IOCON->PIO0_8=2|DIGITAL;

	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<7);

	LPC_TMR16B0->MCR = 3<<3;
	LPC_TMR16B0->PWMC|=(1<<0); //MRO0 = pwm

	LPC_TMR16B0->MR0=1;
	LPC_TMR16B0->MR1=1;

	NVIC_DisableIRQ(TIMER_16_0_IRQn);
	enable_timer16(0);
}

void screenSend2pixel(int8_t pix[2],int x, int y)
{

	/*const uint8_t screenpower[WIDTH][HEIGHT]={{7,9,13,14,14,14}};
	const int8_t screenoffset[WIDTH][HEIGHT]={{0,-2,-4,-7,-7,-7}};//pwm value for 0 mangetic field (electromanget=-aiment)*/

	//const uint8_t screenpower[WIDTH][HEIGHT]={{6,6,6,6,6,6}};
	//const int8_t screenoffset[WIDTH][HEIGHT]={{-3,-3,-3,-3,-3,-3}};

	uint8_t data=0; //3bit bobine ssp in 6bit
	for(int l=0;l<2;l++)
	{
		int temp;
		temp=pix[l];
		//temp=temp*screenpower[x][y]/6+screenoffset[x][y];
		//temp-=3;
		temp*=screendir[x][y];
		if(temp>3)
			temp=3;
		if(temp<-3)
			temp=-3;

		if(power<LIMITE_POWER)
		{
			uint32_t powerTemp=abs(temp);
			if(powerTemp>3)
				powerTemp=3;
			power+=powerTemp*powerTemp*10;//in mA at 5V
		}
		else
		{
			temp=0;
		}
		if(temp<0)
		{
			temp=-temp;
		}
		else
		{
			temp=temp|(1<<2);
		}
		if(l%2==0)
		{
			data=temp<<3;
		}
		else
		{
			data|=temp;
		}
		y++;
	}
	SSP_Send_not_bloking(0, data);
}

void screenUpdate()
{
	int equal=0;
	while(!equal)
	{
		equal=1;
		for(int x=0;x<WIDTH;x++)
		{
			for(int y=0;y<HEIGHT;y++)
			{
				currentScreen[x][y]=screen[x][y];
			}
		}
	}
}

void cleanScreen()
{
	int pos=timeS/4%((WIDTH-4)*2);
	if(pos>WIDTH-4)
		pos=(WIDTH-4)*2-pos;
	for(int y=0;y<HEIGHT;y++)
	{
		for(int x=0;x<WIDTH;x++)
		{
			screen[x][y]=-1;
		}
		screen[pos][y]=3;
		screen[pos+1][y]=3;
		screen[pos+2][y]=3;
		screen[pos+3][y]=3;
	}
}

void screenBare()
{
	for(int x=0;x<WIDTH;x++)
	{
		for(int y=0;y<HEIGHT/2;y++)
		{
			screen[x][y]=-1;
		}
		for(int y=HEIGHT/2;y<HEIGHT;y++)
		{
			screen[x][y]=2;
		}
	}
}

void printTime(uint8_t h,uint8_t m,uint8_t s)
{
	static uint8_t oldH=99;
	static uint8_t oldM=99;
	for(int x=0;x<WIDTH;x++)
	{
		screen[x][HEIGHT-1]=0;
	}

	for(int y=0;y<HEIGHT-1;y++)
	{
		screen[3][y]=-1;
		screen[7][y]=-1;
		screen[8][y]=-1;
		screen[9][y]=-1;
		screen[13][y]=-1;
	}

	if(s&0x01)
	{
		screen[8][1]=3;
		screen[8][5]=3;
		screen[8][2]=1;
		screen[8][4]=1;
	}
	else
	{
		screen[8][1]=1;
		screen[8][5]=1;
		screen[8][2]=3;
		screen[8][4]=3;
	}

	if(s<1)
	{
		if(m%2)
		{
			writeChar(screen, 0, ':',1);
			writeChar(screen, 4, ':',1);
			writeChar(screen, 10, ':',1);
			writeChar(screen, 14, ':',1);
		}
		else
		{
			writeChar(screen, 0, ';',1);
			writeChar(screen, 4, ';',1);
			writeChar(screen, 10, ';',1);
			writeChar(screen, 14, ';',1);
		}
	}
	else if(s<3)
	{
		writeChar(screen, 0, (uint8_t)'0'+(m%10),3);
		writeChar(screen, 4, (uint8_t)'0'+(m/10)%10,3);
		writeChar(screen, 10, (uint8_t)'0'+(h%10),3);
		writeChar(screen, 14, (uint8_t)'0'+(h/10)%10,3);
	}
	else
	{
		writeChar(screen, 0, (uint8_t)'0'+(m)%10,1);
		writeChar(screen, 4, (uint8_t)'0'+(m/10)%10,1);

		writeChar(screen, 10, (uint8_t)'0'+(h)%10,1);
		writeChar(screen, 14, (uint8_t)'0'+(h/10)%10,1);

		oldM=m;
		oldH=h;
	}
	newData=1;
}

void screenUpdateSmooth()
{
	for(int x=0;x<WIDTH;x++)
	{
		for(int y=0;y<HEIGHT;y++)
		{
			if(currentScreen[x][y]<screen[x][y])
			{
				currentScreen[x][y]+=2;
				if(currentScreen[x][y]>screen[x][y])
					currentScreen[x][y]=screen[x][y];
			}
			else if(currentScreen[x][y]>screen[x][y])
			{
				currentScreen[x][y]-=1;
				if(currentScreen[x][y]<screen[x][y])
					currentScreen[x][y]=screen[x][y];
			}
		}
	}
}

void screenComputeMs()
{
	static int x=0;
	static int y=-2;
	static int wait=0;



	if(wait)// wait betwhen 2 trame
	{
		wait--;
		if(wait==0)//prepare next text
		{
			//newData=1;
			/*static char cpt;
			cpt++;
			if(cpt%7==0)
			{
				writeChar(screen, 0, (uint8_t)'0'+(cpt/7)%10);
			}
			else if(cpt%7==6)
			{
				writeChar(screen, 0, 10);
			}

			newData++;*/
		}
		power=0;
	}
	else if(newData && SSP_Ready_for_send(0))
	{
		screenSend2pixel(&(screen[x][y]),x,y);
		y+=2;
		if(y==HEIGHT)
		{
			y=0;
			x++;
			if(x==WIDTH)
			{
				if(smooth)
				{
					screenUpdateSmooth();
				}
				else
				{
					screenUpdate();
				}
				x=0;
				wait=70+smooth;
				newData--;
			}
		}

	}

}
