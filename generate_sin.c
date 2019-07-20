#include "math.h"
#include "stdio.h"

#define NB_POINT_SIN 20

void main()
{
	int l;
	printf("#define NB_POINT_SIN %d\n\n",NB_POINT_SIN);
	printf("const signed char SIN[%d]={",NB_POINT_SIN);
	for(l=0;l<NB_POINT_SIN;l++)
	{
		printf("%d",(int)(sin(2*M_PI/(float)NB_POINT_SIN*l)*100));
		if(l<NB_POINT_SIN-1)
			printf(",");
	}
	printf("};\n",NB_POINT_SIN);

	
}
