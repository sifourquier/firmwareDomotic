/*
 * pwm.h
 *
 *  Created on: Nov 16, 2018
 *      Author: simon
 */

#ifndef PWM_H_
#define PWM_H_
#define MAX_POWER 1000
#define DIV_FREQU_PWM 48

extern volatile uint64_t time; //time in 1/1'000'000 seconde
extern volatile uint32_t timeS;//time in s
extern volatile uint8_t flagNewSec;//set every seconde
extern volatile uint8_t flagNew1ms;

void pwmInit();
void setPwm(uint8_t numPwm,uint16_t val);

#endif /* PWM_H_ */
