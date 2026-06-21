/*
* pwm.h
*
* Created: 2026-06-18 오후 2:22:34
*  Author: kccistc
*/


#ifndef PWM_H_
#define PWM_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>

void init_timer_3_pwm(void);
void init_motor_driver(void);
void motor_set_speed(int speed);

#endif /* PWM_H_ */
