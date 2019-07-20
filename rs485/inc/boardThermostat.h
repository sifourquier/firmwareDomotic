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

#define PORT_SW3 3
#define BIT_SW3 4

#define PORT_SW4 1
#define BIT_SW4 11

#define PORT_SW5 3
#define BIT_SW5 5

#define PORT_RX 1
#define BIT_RX 6

#define PORT_TX 1
#define BIT_TX 7

#define EINT_TRIAC EINT0_IRQn

#define PORT_LED1 1
#define BIT_LED1 8

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

#define PORT_nCS_LCD 1
#define BIT_nCS_LCD 10

#define PORT_nWR_LCD 0
#define BIT_nWR_LCD 2

#define PORT_nRD_LCD 0
#define BIT_nRD_LCD 3

#define PWM_BACKLIGHT 1

#ifdef THERMOSTAT_LCD
#define NB_SW 5
#define LIST_PORT_SW PORT_SW3,PORT_SW4,PORT_SW2,PORT_SW1,PORT_SW5
#define LIST_BIT_SW BIT_SW3,BIT_SW4,BIT_SW2,BIT_SW1,BIT_SW5
#else
#define NB_SW 4
#define LIST_PORT_SW PORT_SW1,PORT_SW2,PORT_SW3,PORT_SW4
#define LIST_BIT_SW BIT_SW1,BIT_SW2,BIT_SW3,BIT_SW4
#endif

#define PORT_DTH22 0
#define BIT_DTH22 5
#define GPIO_PORT_DTH22 LPC_GPIO0
#define EINT_DHT22 EINT0_IRQn

#define PWM_ORDER 1,0,2

#define INIT_LED_SWITCH LPC_IOCON->R_PIO1_0=1|DIGITAL

#define PORT_LED_SWITCH 1
#define BIT_LED_SWITCH 0

#define LCD_UP_BUTTON 0
#define LCD_MOD_BUTTON 1
#define LCD_TIME_BUTTON 2
#define LCD_POWER_BUTTON 3
#define LCD_DOWN_BUTTON 4
#define initBoard()\
		SET_OUT(PORT_PWM1,BIT_PWM1);\
		SET_OUT(PORT_PWM2,BIT_PWM2);\
		SET_OUT(PORT_PWM3,BIT_PWM3);\
		SET_OUT(PORT_TX,BIT_TX);\
		SET_OUT(PORT_LED1,BIT_LED1);\
		SET_OUT(PORT_SCLK,BIT_SCLK);\
		SET_OUT(PORT_MOSI,BIT_MOSI);\
		SET_OUT(PORT_nRE,BIT_nRE);\
		SET_OUT(PORT_DE,BIT_DE);\
		SET_OUT(PORT_nCS_LCD,BIT_nCS_LCD);\
		SET_OUT(PORT_nWR_LCD,BIT_nWR_LCD);\
		SET_BIT(PORT_nWR_LCD,BIT_nWR_LCD,1);/*disable connecte to MOSI to MOSI*/\
		SET_OUT(PORT_nRD_LCD,BIT_nRD_LCD);\
		SET_BIT(PORT_nRD_LCD,BIT_nRD_LCD,1);/*default write*/\
		LPC_IOCON->PIO3_5=PULLUP|DIGITAL;/*sw5*/\
		LPC_IOCON->PIO0_2=DIGITAL|OPENDRAIN;/*nWR LCD*/\
		LPC_IOCON->PIO0_3=DIGITAL|OPENDRAIN;/*nRD LCD*/\
		LPC_IOCON->PIO0_4=1;/*SCL*/\
		LPC_IOCON->PIO0_5=1;/*SDA*/\
		LPC_IOCON->SCK_LOC=2;/*SCK0 on PIO0_6*/\
		LPC_IOCON->PIO0_6=2|DIGITAL|OPENDRAIN;/*SCK0*/\
		/*LPC_IOCON->PIO0_7=0;/*NC*/\
		LPC_IOCON->PIO0_8=1|DIGITAL|OPENDRAIN;/*MISO0*/\
		LPC_IOCON->PIO0_9=1|DIGITAL|OPENDRAIN;/*MOSI0*/\
		LPC_IOCON->R_PIO0_11=1|DIGITAL;/*pwm3*/\
		/*LPC_IOCON->R_PIO1_0=1|PULLUP|DIGITAL;/*NC*/\
		LPC_IOCON->R_PIO1_1=3|DIGITAL;/*pwm1*/\
		LPC_IOCON->R_PIO1_2=3|DIGITAL;/*pwm2*/\
		LPC_IOCON->PIO1_4=PULLUP|DIGITAL;/*sw1*/\
		LPC_IOCON->PIO1_8=DIGITAL;/*LED*/\
		LPC_IOCON->PIO1_9=PULLUP|DIGITAL;/*sw2*/\
		LPC_IOCON->PIO1_10=DIGITAL|OPENDRAIN;/*nCS LCD*/\
		LPC_IOCON->PIO1_11=PULLUP|DIGITAL;/*sw4*/\
		LPC_IOCON->PIO2_0=0;/*RE*/\
		LPC_IOCON->PIO3_2=0;/*DE*/\
		LPC_IOCON->PIO3_2=0|DIGITAL;
#endif