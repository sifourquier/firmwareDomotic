#include "charSet.h"

#define F 2
#define N -1
#define _ 0
int8_t charSet[11][6][3]=
{
		{ //0
				{F,F,F},
				{F,N,F},
				{F,N,F},
				{F,N,F},
				{F,F,F},
				{_,_,_}
		},
		{ //1
				{3,3,N},
				{N,3,N},
				{N,3,N},
				{N,3,N},
				{3,3,3},
				{_,_,_}
		},
		{ //2
				{F,F,F},
				{N,N,F},
				{F,F,F},
				{F,N,N},
				{F,F,F},
				{_,_,_}
		},
		{ //3
				{F,F,F},
				{N,N,F},
				{F,F,F},
				{N,N,F},
				{F,F,F},
				{_,_,_}
		},
		{ //4
				{F,N,F},
				{F,N,F},
				{F,F,F},
				{0,N,F},
				{N,N,F},
				{_,_,_}
		},
		{ //5
				{F,F,F},
				{F,N,N},
				{F,F,F},
				{N,N,F},
				{F,F,F},
				{_,_,_}
		},
		{ //6
				{F,F,F},
				{F,N,N},
				{F,F,F},
				{F,N,F},
				{F,F,F},
				{_,_,_}
		},
		{ //7
				{F,F,F},
				{N,N,F},
				{N,N,F},
				{N,N,F},
				{N,N,F},
				{_,_,_}
		},
		/*{ //7
				{F,F,F},
				{N,N,F},
				{N,F,F},
				{F,F,N},
				{F,N,N},
				{_,_,_}
		},*/
		{ //8
				{F,F,F},
				{F,N,F},
				{F,F,F},
				{F,N,F},
				{F,F,F},
				{_,_,_}
		},
		{ //9
				{F,F,F},
				{F,N,F},
				{F,F,F},
				{N,N,F},
				{F,F,F},
				{_,_,_}
		},
		{ //10 |
			{N,3,N},
			{N,3,N},
			{N,3,N},
			{N,3,N},
			{N,3,N},
			{_,_,_}
		}
};

void writeChar(int8_t screen[WIDTH][HEIGHT], int pos, uint8_t c, int power)
{
	pos=WIDTH-1-pos;
	c=c-'0';
	if(c>10)
		c=10;
	for(int l=0;l<3;l++)
	{
		for(int l2=0;l2<6;l2++)
		{
			int32_t temp=charSet[c][l2][2-l];
			if(temp>0)
			{
				screen[pos+2-l][l2]=temp*power;
			}
			else
			{
				screen[pos+2-l][l2]=temp;
			}
		}
	}
}
