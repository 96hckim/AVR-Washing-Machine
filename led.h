/**
* @file    led.h
* @brief   ATmega128A PORTA 기반 LED 저수준 제어 및 시퀀스 패턴 드라이버 인터페이스
* @date    2026-06-11
* @author  kccistc
*/

#ifndef LED_H_
#define LED_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>

#define LED_DDR		DDRA
#define LED_PORT	PORTA

void init_led(void);
void led_all_off(void);
void led_on(int index);

#endif /* LED_H_ */