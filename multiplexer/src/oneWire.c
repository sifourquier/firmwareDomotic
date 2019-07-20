#include "timer16.h"
#include "board.h"
#include "oneWire.h"
#include "gpio.h"

#define TIME1W 60

typedef enum
{
	S1W_RESET,
	S1W_WAIT_SLAVE,
	S1W_REPLY_SLAVE,
	S1W_WAIT,
	S1W_WRITE,
	S1W_READ,
	S1W_END,
}t_state1wire;

typedef enum
{
	S1H_START_LOW,
	S1H_START_HIGH,
	S1H_ACK,
	S1H_ACK1,
	S1H_READ,
	S1H_END,
}t_stateDHT22;

typedef enum
{
	S1WM_1WIRE,
	S1WM_DHT22,
	S1WM_END,
}t_mode1wire;

static uint8_t data1wireWr[MAX_LENGTH_1_WIRE]={0};
static uint8_t data1wireRd[MAX_LENGTH_1_WIRE]={0};

static volatile uint16_t pos_data=0;
static volatile uint8_t lentgh_write=0;
static volatile uint8_t lentgh_read=0;

static t_state1wire state_1W=S1W_END;
static t_stateDHT22 state_DHT=S1H_END;

static volatile uint64_t dataDHT22;
static volatile uint8_t posDataDHT22;
static volatile uint16_t timeDHT22;

static t_mode1wire mode=S1WM_END;

int SendReadOneWire(uint8_t* dataWr,uint8_t lengthWr,uint8_t lengthRd)
{
	if(state_1W!=S1W_END || mode!=S1WM_END)
		return -1;
	mode=S1WM_1WIRE;
	if(lengthWr>MAX_LENGTH_1_WIRE)
		lengthWr=MAX_LENGTH_1_WIRE;
	if(lengthRd>MAX_LENGTH_1_WIRE)
		lengthRd=MAX_LENGTH_1_WIRE;
	lentgh_read=lengthRd;
	lentgh_write=lengthWr;
	for(int l=0;l<lengthWr;l++)
		data1wireWr[l]=dataWr[l];
	for(int l=0;l<MAX_LENGTH_1_WIRE;l++)
		data1wireRd[l]=0;
	state_1W=S1W_RESET;
	reset_timer16(0);
	enable_timer16(0);
	return 1;
}

int getDataOneWire(uint8_t ** buff)
{
	if(state_1W!=S1W_END)
	{
		return -1;
	}
	if(buff)
		*buff=data1wireRd;
	return lentgh_read;
}

void initOneWire()
{
	init_timer16(0, TIME1W);
	//reset_timer16(0);
	//enable_timer16(0);
	SET_BIT(PORT_1WIRE,BIT_1WIRE,0);
	SET_OUT(PORT_1WIRE,BIT_1WIRE);
}

#define wait1us() {uint16_t t0=LPC_TMR16B0->TC; while(LPC_TMR16B0->TC-t0<48);}

void TIMER16_0_IRQHandler(void)
{
	if(mode==S1WM_1WIRE)
	{
		uint8_t read;
		switch (state_1W) {
		case S1W_RESET:
			SET_OUT(PORT_1WIRE,BIT_1WIRE);
			SET_BIT(PORT_1WIRE,BIT_1WIRE,0);
			pos_data++;
			if(pos_data>=9)
				state_1W=S1W_WAIT_SLAVE;
			break;
		case S1W_WAIT_SLAVE:
			SET_BIT(PORT_1WIRE,BIT_1WIRE,1);
			SET_IN(PORT_1WIRE,BIT_1WIRE);
			pos_data=0;
			state_1W=S1W_REPLY_SLAVE;
			break;
		case S1W_REPLY_SLAVE:
			read=GET_BIT(PORT_1WIRE,BIT_1WIRE);
			//SET_OUT(PORT_1WIRE,BIT_1WIRE);
			//SET_BIT(PORT_1WIRE,BIT_1WIRE,1);
			state_1W=S1W_WAIT;
			break;
		case S1W_WAIT:
			pos_data++;
			if(pos_data>=3)
			{
				pos_data=0;
				state_1W=S1W_WRITE;
			}
			break;
		case S1W_WRITE:
			SET_OUT(PORT_1WIRE,BIT_1WIRE);
			SET_BIT(PORT_1WIRE,BIT_1WIRE,1);
			wait1us();
			SET_BIT(PORT_1WIRE,BIT_1WIRE,0);
			wait1us();
			if((data1wireWr[pos_data/8]>>(pos_data&0x7))&0x01)
			{
				SET_BIT(PORT_1WIRE,BIT_1WIRE,1);
			}
			pos_data++;
			if(pos_data>=lentgh_write*8)
			{
				pos_data=0;
				state_1W=S1W_READ;
			}
			break;
		case S1W_READ:
			SET_OUT(PORT_1WIRE,BIT_1WIRE);
			SET_BIT(PORT_1WIRE,BIT_1WIRE,1);
			wait1us();
			SET_BIT(PORT_1WIRE,BIT_1WIRE,0);
			wait1us();
			SET_BIT(PORT_1WIRE,BIT_1WIRE,1);
			SET_IN(PORT_1WIRE,BIT_1WIRE);
			wait1us();
			read=GET_BIT(PORT_1WIRE,BIT_1WIRE);
			//SET_OUT(PORT_1WIRE,BIT_1WIRE);
			//SET_BIT(PORT_1WIRE,BIT_1WIRE,1);

			if(read)
				data1wireRd[pos_data/8]|=1<<pos_data%8;
			pos_data++;
			if(pos_data>=lentgh_read*8)
			{
				pos_data=0;
				disable_timer16(0);
				state_1W=S1W_END;
				mode=S1WM_END;
			}
			break;

		default:
			break;
		}
	}
	else if(mode==S1WM_DHT22)
	{
		switch (state_DHT) {
		case S1H_START_LOW:
			timeDHT22++;
			if(timeDHT22>(18000/TIME1W))
			{
				state_DHT=S1H_START_HIGH;
				SET_BIT(PORT_DTH22,BIT_DTH22,1);
				SET_IN(PORT_DTH22,BIT_DTH22);
			}
			break;
		case S1H_START_HIGH:
			state_DHT=S1H_ACK;
			break;
		case S1H_ACK:
			state_DHT=S1H_ACK1;
			break;
		case S1H_ACK1:
			GPIOIntEnable(PORT_DTH22,BIT_DTH22);
			dataDHT22=0;
			state_DHT=S1H_READ;
			break;
		case S1H_READ:
			dataDHT22<<=1;
			dataDHT22|=GET_BIT(PORT_DTH22,BIT_DTH22);
			posDataDHT22++;
			disable_timer16(0);
			if(posDataDHT22>=40)
			{
				GPIOIntDisable(PORT_DTH22,BIT_DTH22);
				state_DHT=S1H_END;
				mode=S1WM_END;
			}
			break;
		case S1H_END:
			break;
		}
	}

	LPC_TMR16B0->IR = 1;
}

int startDHT22()
{
	if((state_DHT!=S1H_END) || (mode!=S1WM_END))
		return -1;

	timeDHT22=0;
	posDataDHT22=0;
	mode=S1WM_DHT22;
	state_DHT=S1H_START_LOW;
	NVIC_EnableIRQ(EINT_DHT22);
	GPIOSetInterrupt(PORT_DTH22,BIT_DTH22, 0, 0, 1);
	SET_OUT(PORT_DTH22,BIT_DTH22);
	SET_BIT(PORT_DTH22,BIT_DTH22,0);
	enable_timer16(0);
	return 1;
}

int getDataDHT22(t_DHT22Data* data)
{
	if(state_DHT!=S1H_END)
		return -1;

	data->humidity=((dataDHT22>>24)&0xFFFF)*10;
	data->temperature=((dataDHT22>>8)&0xFFFF)*10;

	return 1;
}

static int test=0;
void PIOINT1_IRQHandler()
{
	test++;
	reset_timer16(0);
	enable_timer16(0);

	GPIOIntClear(PORT_DTH22,BIT_DTH22);
}


