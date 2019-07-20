#include <LPC11xx.h>

#ifndef BOARD_H_
#define BOARD_H_

#define PULLUP (1<<4)
#define DIGITAL (1<<7)
#define OPENDRAIN (1<<10)
#ifdef BOARD_THERMOSTAT
#include <boardThermostat.h>
#else
#include <boardUniversalV2.h>
#endif

#define LPC_GPIO_TAB ((LPC_GPIO_TypeDef*)LPC_GPIO_BASE)

#define SET_OUT(PORT,BIT) ((LPC_GPIO_TypeDef*)(LPC_GPIO_BASE+0x10000*PORT))->DIR|=1<<BIT;
#define SET_IN(PORT,BIT) ((LPC_GPIO_TypeDef*)(LPC_GPIO_BASE+0x10000*PORT))->DIR&=~(1<<BIT);
#define SET_BIT(PORT,BIT,x) ((LPC_GPIO_TypeDef*)(LPC_GPIO_BASE+0x10000*PORT))->MASKED_ACCESS[1<<BIT]=((x ? 1 : 0)<<BIT);
#define GET_BIT(PORT,BIT) ((((LPC_GPIO_TypeDef*)(LPC_GPIO_BASE+0x10000*PORT))->DATA>>BIT)&1)

#endif
