/*
* fnd.c
*
* Created: 2026-06-12 오전 10:52:34
* Author: kccistc
*/

#include "fnd.h"

// 숫자 폰트 정의
#if FND_CONFIG_ANODE
static const uint8_t fnd_font[] = { 0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xD8, 0x80, 0x98, 0x7F };
#else
static const uint8_t fnd_font[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x27, 0x7F, 0x67, 0x80 };
#endif

// 회전 애니메이션 데이터 테이블
#if FND_CONFIG_ANODE
static const uint8_t circle_anime[][2] = {
	{ 0xFB, FND_DIGIT_100 },   // 0: 100의 자리 c
	{ 0xFD, FND_DIGIT_100 },   // 1: 100의 자리 b
	{ 0xFE, FND_DIGIT_100 },   // 2: 100의 자리 a
	{ 0xFE, FND_DIGIT_1000 },  // 3: 1000의 자리 a
	{ 0xDF, FND_DIGIT_1000 },  // 4: 1000의 자리 f
	{ 0xEF, FND_DIGIT_1000 },  // 5: 1000의 자리 e
	{ 0xF7, FND_DIGIT_1000 },  // 6: 1000의 자리 d
	{ 0xF7, FND_DIGIT_100 }    // 7: 100의 자리 d
};
#else
static const uint8_t circle_anime[][2] = {
	{ ~0xFB, FND_DIGIT_100 },   // 0: 100의 자리 c
	{ ~0xFD, FND_DIGIT_100 },   // 1: 100의 자리 b
	{ ~0xFE, FND_DIGIT_100 },   // 2: 100의 자리 a
	{ ~0xFE, FND_DIGIT_1000 },  // 3: 1000의 자리 a
	{ ~0xDF, FND_DIGIT_1000 },  // 4: 1000의 자리 f
	{ ~0xEF, FND_DIGIT_1000 },  // 5: 1000의 자리 e
	{ ~0xF7, FND_DIGIT_1000 },  // 6: 1000의 자리 d
	{ ~0xF7, FND_DIGIT_100 }    // 7: 100의 자리 d
};
#endif

// 자릿수 제어 핀 배열
static const uint8_t digit_pins[4] = { FND_DIGIT_1, FND_DIGIT_10, FND_DIGIT_100, FND_DIGIT_1000 };

void init_fnd(void)
{
	FND_DATA_DDR = 0xFF;
	FND_DIGIT_DDR |= FND_DIGIT_MASK;
	
	fnd_data_all_off();
	fnd_digit_all_off();
}

void fnd_data_all_off(void)
{
	#if FND_CONFIG_ANODE
	FND_DATA_PORT = 0xFF;
	#else
	FND_DATA_PORT = 0x00;
	#endif
}

void fnd_digit_all_off(void)
{
	#if FND_CONFIG_ANODE
	FND_DIGIT_PORT &= ~FND_DIGIT_MASK;
	#else
	FND_DIGIT_PORT |= FND_DIGIT_MASK;
	#endif
}

void fnd_digit_on(uint8_t pin)
{
	#if FND_CONFIG_ANODE
	FND_DIGIT_PORT = 1 << (pin);
	#else
	FND_DIGIT_PORT = ~(1 << (pin));
	#endif
}

void display_setting_mode(uint8_t sec, uint8_t min)
{
	static uint8_t pin = 0;
	
	uint8_t display_number = 0;
	
	switch (pin)
	{
		case 0:
		display_number = min % 10;
		break;
		
		case 1:
		display_number = min / 10;
		break;
		
		default:
		break;
	}
	
	FND_DATA_PORT = fnd_font[display_number];
	fnd_digit_on(digit_pins[pin]);
	
	pin = (pin + 1) % 2;
}

void display_running_mode(uint8_t sec, uint8_t min)
{
	static uint8_t pin = 0;
	static uint16_t anime_tick_count = 0;
	static uint8_t anime_index = 0;
	
	anime_tick_count++;
	if (anime_tick_count >= 100)
	{
		anime_tick_count = 0;
		anime_index = (anime_index + 1) % 8;
	}
	
	uint8_t target_number = (min > 0) ? min : sec;
	
	switch (pin)
	{
		case 0:
		FND_DATA_PORT = fnd_font[target_number % 10];
		fnd_digit_on(digit_pins[pin]);
		break;
		
		case 1:
		FND_DATA_PORT = fnd_font[target_number / 10];
		fnd_digit_on(digit_pins[pin]);
		break;

		case 2:
		case 3:
		FND_DATA_PORT = circle_anime[anime_index][0];
		fnd_digit_on(circle_anime[anime_index][1]);
		break;
		
		default:
		break;
	}
	
	pin = (pin + 1) % 4;
}
