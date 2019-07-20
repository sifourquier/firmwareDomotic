#include "modbus.h"
#include "LPC11xx.h"
#include "uart.h"
#include "timer32.h"
#include "stdio.h"
#include "main.h"
#include "firmware.h"
#include "board.h"

#define NB_MAX_IN 32

typedef enum
{
	Smb_WAIT,
	Smb_address,
	Smb_FUNCTION,
	Smb_DATA
} t_Smb;

/*typedef struct
{
	uint16_t value;
	uint8_t reset; //reset value after read
	uint8_t is_read;
} t_in;

t_in in[NB_MAX_IN];*/

static uint16_t wCRCTable[] = {
0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };

static t_paquet paquet={0,0,0};
static t_Smb state=Smb_address;
static uint32_t numByteaddress=0;
volatile uint32_t modeBusaddress=3;

void modeBusUartRx(uint8_t c);

modbus_write_cb_t callBackWrite=NULL;
modbus_read_cb_t callBackRead=NULL;

t_paquet paquetToSend;
uint8_t flagSendPaquet;

void modBusSetCallbackWrite(modbus_write_cb_t l_callBackWrite)
{
	callBackWrite=l_callBackWrite;
}

void modBusSetCallbackRead(modbus_read_cb_t l_callBackRead)
{
	callBackRead=l_callBackRead;
}

#define SET_RECEIVE SET_BIT(PORT_DE,BIT_DE,0); SET_BIT(PORT_nRE,BIT_nRE,0);
#define SET_TRANSMIT SET_BIT(PORT_DE,BIT_DE,1); SET_BIT(PORT_nRE,BIT_nRE,1);


void modeBusInit()
{
	SET_RECEIVE;
	UartInit(BAUDDRATE);
	
	init_timer32(0, ((SystemCoreClock/LPC_SYSCON->SYSAHBCLKDIV)/((BAUDDRATE/12.5)))); //10byte +2.5 (3.5 betwen 2 paquet)
	NVIC_SetPriority(TIMER_32_0_IRQn,0);//highest priority
	disable_timer32(0);
	UartSetCallback(modeBusUartRx);
}

void modeBusUartRx(uint8_t c)
{
	reset_timer32(0);
	enable_timer32(0);
	switch(state)
	{
		case Smb_WAIT://state change in TIMER32_0_IRQHandler
			numByteaddress=0;
			break;
		case Smb_address:
			paquet.address<<=8;
			paquet.address=(paquet.address&(~0xFF))|c;
			numByteaddress++;
			if(numByteaddress==ADDRESS_LENGTH)
			{
				if(paquet.address==modeBusaddress)
					state=Smb_FUNCTION;
				else
					state=Smb_WAIT;
			}
			break;
		case Smb_FUNCTION:
			paquet.length=0;
			paquet.function=c;
			state=Smb_DATA;
			break;
		case Smb_DATA:
			if(paquet.length<MAX_PAQUET_SIZE)
			{
				paquet.data[paquet.length]=c;
				paquet.length++;
			}
			break;
	}
}

/*
 * uint16_t crc_modbus( const unsigned char *input_str, size_t num_bytes );
 *
 * The function crc_modbus() calculates the 16 bits Modbus CRC in one pass for
 * a byte string of which the beginning has been passed to the function. The
 * number of bytes to check is also a parameter.
 */
void modeBusCrc( uint8_t *input_str, uint32_t num_bytes, uint16_t *crc)
{

	uint16_t tmp;
	uint16_t short_c;
	const unsigned char *ptr;
	uint32_t a;

	ptr = input_str;

	if ( ptr != 0 ) for (a=0; a<num_bytes; a++) {
		short_c = 0x00ff & (uint16_t) *ptr;
		tmp     =  *crc       ^ short_c;
		*crc     = (*crc >> 8) ^ wCRCTable[ tmp & 0xff ];
		ptr++;
	}
}  /* crc_modbus */

void modeBusSendPaquet(t_paquet paquet)
{
	uint8_t buff[256];
	int p=0;
	for(int l=0;l<ADDRESS_LENGTH;l++)
	{
		buff[p]=paquet.address>>((ADDRESS_LENGTH-l-1)*8);
		p++;
	}
	buff[p]=paquet.function;
	p++;
	for(int l=0;l<paquet.length;l++)
	{
		buff[p]=paquet.data[l];
		p++;
	}

	uint16_t crc=CRC_INIT_VALUE;
	modeBusCrc(buff,p,&crc);
	buff[p]=crc;
		p++;
	buff[p]=crc>>8;
		p++;
	SET_TRANSMIT;
	UartSend(buff,p);
	while ( !(LPC_UART->LSR & LSR_TEMT) );//wait end of transmit
	SET_RECEIVE;
}

void modeBusPaquetRead(t_paquet paquet)
{
	uint16_t dataAddress=(paquet.data[0]<<8)|paquet.data[1];
	uint16_t nbValue=1;
	uint16_t posValue=2;
	uint16_t LengthRead=0;
	uint8_t nbByte=0;
	uint8_t nbByteByVal=2;
	uint64_t val_bool;
	switch (paquet.function)
	{
		case F_READ_COIL:
		case F_READ_INPUT:
		case F_READ_INPUT_REG: //nb_reads =1
		case F_READ_REG:
			nbValue=(paquet.data[2]<<8)|paquet.data[3];

			for(int l=0;l<nbValue;l++)
			{
				if(callBackRead)
				{
					LengthRead+=callBackRead(paquet.function,dataAddress,paquetToSend.data+LengthRead/8+1);
				}
				dataAddress++;
			}
			paquetToSend.data[0]=((LengthRead+7)/8)*1;
			paquetToSend.length=paquetToSend.data[0]+1;
			paquetToSend.address=paquet.address;
			paquetToSend.function=paquet.function;
			flagSendPaquet=1;
			break;
		case F_WRITE_COILS:
		case F_WRITE_REGS:
			nbValue=(paquet.data[2]<<8)|paquet.data[3];
			posValue=4;
			nbByte=paquet.data[posValue];
			if(paquet.function==F_WRITE_REGS)
				nbByteByVal=nbByte/nbValue;
			posValue++;
		case F_WRITE_REG:
		case F_WRITE_COIL:
			val_bool=0;
			if((paquet.function==F_WRITE_COIL) || (paquet.function==F_WRITE_COILS))
			{
				for(int l=0;l<nbByteByVal;l++)
				{
					//val_bool<<=8;
					val_bool|=paquet.data[posValue]<<(8*l);
					posValue++;
				}
			}
			if(dataAddress==MOD_BUS_ADDRESS_FIRMWARE && paquet.function==F_WRITE_REGS)
			{
				firmwareDecode(paquet.data+posValue,nbValue);
			}
			else
			{
				for(int l=0;l<nbValue;l++)
				{

					uint32_t value;
					if((paquet.function==F_WRITE_COIL) || (paquet.function==F_WRITE_COILS))
					{
						value=((val_bool>>dataAddress) & 0x01)*0xFFFFFFFF;
					}
					else
					{
						value=0;
						for(int l=0;l<nbByteByVal;l++)
						{
							value<<=8;
							value|=paquet.data[posValue];
							posValue++;
						}
					}
					if(callBackWrite)
						callBackWrite(paquet.function,dataAddress,value);
					dataAddress++;
				}
			}
			paquetToSend=paquet; //reply same paquet truncat for only send 4byte (adresse + length) for F_WRITE_COILS F_WRITE_ANS
			paquetToSend.length=4;
			flagSendPaquet=1;
			break;
	}
}

void TIMER32_0_IRQHandler(void)
{
	disable_timer32(0);
	if(paquet.length>=5)
	{
		paquet.length-=2; //remove crc
		uint16_t crc_paquet=paquet.data[paquet.length]|(paquet.data[paquet.length+1]<<8);
		uint16_t crc=CRC_INIT_VALUE;
		uint8_t temp=paquet.address;
		modeBusCrc(&temp,1,&crc);
		temp=paquet.function;
		modeBusCrc(&temp,1,&crc);
		modeBusCrc(paquet.data,paquet.length,&crc);
		if(crc==crc_paquet)
		{
			modeBusPaquetRead(paquet);
		}
	}

	state=Smb_address;
	numByteaddress=0;
	paquet.address=0;
	LPC_TMR32B0->IR = 1;			/* clear interrupt flag */
	return;
}

void modeBusCompute()
{
	if(flagSendPaquet)
	{
		modeBusSendPaquet(paquetToSend);
		flagSendPaquet=0;
	}
}
