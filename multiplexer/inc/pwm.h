/*
 * pwm.h
 *
 *  Created on: Nov 16, 2018
 *      Author: simon
 */

#ifndef PWM_H_
#define PWM_H_

#define MAX_POWER 1000
#define DIV_FREQU_PWM 100

extern volatile uint64_t time; //time in 1/1'000'000 seconde
extern volatile uint64_t time_s;//time in s
extern volatile uint8_t flag_new_sec;//set every seconde

void init_pwm();
void setPwm(uint8_t numPwm,uint16_t val);

#endif /* PWM_H_ */
