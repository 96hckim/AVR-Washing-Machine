/**
* @file    led.c
* @brief   ATmega128A PORTA 기반 LED 저수준 제어 및 비트 알고리즘 시퀀스 구현
* @date    2026-06-17
* @author  kccistc
*/

#include "led.h"

void init_led(void)
{
	LED_DDR = 0xFF;  /* PORTA 출력 모드 */
	led_all_off();
}

void led_all_off(void)
{
	LED_PORT = 0x00;
}

void led_on(int index)
{
	if (index < 0 || index > 7)
	{
		led_all_off();
		return;
	}
	
	PORTA = 1 << index;
}
