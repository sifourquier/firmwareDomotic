#ifndef BOARDUNIVERSAL_H_
#define BOARDUNIVERSAL_H_

#define PORT_PWM1 1
#define BIT_PWM1 1

#define PORT_PWM2 1
#define BIT_PWM2 2

#define PORT_PWM3 0
#define BIT_PWM3 11

#define PORT_SW1 1
#define BIT_SW1 4

#define PORT_SW2 1
#define BIT_SW2 9

#define PORT_SW3 1
#define BIT_SW3 8

#define PORT_SW4 1
#define BIT_SW4 11

#define PORT_RX 1
#define BIT_RX 6

#define PORT_TX 1
#define BIT_TX 7

#define PORT_nCS_TLI 3
#define BIT_nCS_TLI 4

#define GPIOPORT_nZERO_CROSS LPC_GPIO0
#define PORT_nZERO_CROSS 0
#define BIT_nZERO_CROSS 3

#define PORT_TRIAC1 0
#define BIT_TRIAC1 4
#define EINT_TRIAC EINT0_IRQn

#define PORT_LED1 0
#define BIT_LED1 5

#define PORT_EN_CURRENT 0
#define BIT_EN_CURRENT 7

#define PORT_SCLK 0
#define BIT_SCLK 6

#define PORT_MISO 0
#define BIT_MISO 8

#define PORT_MOSI 0
#define BIT_MOSI 9

#define PORT_nRE 3 //receive
#define BIT_nRE 2

#define PORT_DE 2 //send
#define BIT_DE 0

#define PORT_1WIRE 1 //send
#define BIT_1WIRE 10

#define NB_SW 4
#define LIST_PORT_SW PORT_SW1,PORT_SW2,PORT_SW3,PORT_SW4
#define LIST_BIT_SW BIT_SW1,BIT_SW2,BIT_SW3,BIT_SW4

#define PORT_DTH22 1
#define GPIO_PORT_DTH22 LPC_GPIO1
#define BIT_DTH22 0
#define EINT_DHT22 EINT1_IRQn

#define PORT_DTH22_2 0
#define BIT_DTH22_2 2

#define PWM_ORDER 1,0,2

#define INIT_LED_SWITCH LPC_IOCON->PIO0_9=0|DIGITAL

#define PORT_LED_SWITCH 0
#define BIT_LED_SWITCH 9


#define initBoard()\
		SET_OUT(PORT_PWM1,BIT_PWM1);\
		SET_OUT(PORT_PWM2,BIT_PWM2);\
		SET_OUT(PORT_PWM3,BIT_PWM3);\
		SET_OUT(PORT_TX,BIT_TX);\
		SET_OUT(PORT_nCS_TLI,BIT_nCS_TLI);\
		SET_OUT(PORT_TRIAC1,BIT_TRIAC1);\
		SET_OUT(PORT_LED1,BIT_LED1);\
		SET_OUT(PORT_EN_CURRENT,BIT_EN_CURRENT);\
		SET_OUT(PORT_SCLK,BIT_SCLK);\
		SET_OUT(PORT_MOSI,BIT_MOSI);\
		SET_OUT(PORT_nRE,BIT_nRE);\
		SET_OUT(PORT_DE,BIT_DE);\
		LPC_IOCON->PIO0_2=0;/*dht*/\
		LPC_IOCON->PIO0_3=0;/*nZeroCross*/\
		LPC_IOCON->PIO0_4=0;/*TRIAC1*/\
		LPC_IOCON->PIO0_5=0;/*led*/\
		LPC_IOCON->SCK_LOC=2;/*SCK0 on PIO0_6*/\
		LPC_IOCON->PIO0_6=2|DIGITAL;/*SCK0*/\
		LPC_IOCON->PIO0_7=0;/*EN_CURRENT*/\
		LPC_IOCON->PIO0_8=1|DIGITAL;/*MISO0*/\
		LPC_IOCON->PIO0_9=1|DIGITAL;/*MOSI0*/\
		LPC_IOCON->R_PIO0_11=1|DIGITAL;/*pwm3*/\
		LPC_IOCON->R_PIO1_0=1|PULLUP|DIGITAL;/*dht22*/\
		LPC_IOCON->R_PIO1_1=3|DIGITAL;/*pwm1*/\
		LPC_IOCON->R_PIO1_2=3|DIGITAL;/*pwm2*/\
		LPC_IOCON->PIO1_4=PULLUP|DIGITAL;/*sw1*/\
		LPC_IOCON->PIO1_8=PULLUP|DIGITAL;/*sw3*/\
		LPC_IOCON->PIO1_9=PULLUP|DIGITAL;/*sw2*/\
		LPC_IOCON->PIO1_10=PULLUP|DIGITAL|OPENDRAIN;/*DS18B20*/\
		LPC_IOCON->PIO1_11=PULLUP|DIGITAL;/*sw4*/\
		LPC_IOCON->PIO2_0=0;/*RE*/\
		LPC_IOCON->PIO3_2=0;/*DE*/\
		LPC_IOCON->PIO3_2=0|DIGITAL;
#endif
