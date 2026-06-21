/*
* fnd.h
*
* Created: 2026-06-12 오전 10:46:00
* Author: kccistc
*/

#ifndef FND_H_
#define FND_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>

// Common Anode: 1 / Common Cathode: 0
#define FND_CONFIG_ANODE    1

#define FND_DATA_DDR        DDRC
#define FND_DATA_PORT       PORTC

#define FND_DIGIT_DDR       DDRB
#define FND_DIGIT_PORT      PORTB

#define FND_DIGIT_1			7
#define FND_DIGIT_10		6
#define FND_DIGIT_100		5
#define FND_DIGIT_1000		4

#define FND_DIGIT_MASK      ((1 << FND_DIGIT_1) | (1 << FND_DIGIT_10) | (1 << FND_DIGIT_100) | (1 << FND_DIGIT_1000))

void init_fnd(void);
void fnd_data_all_off(void);
void fnd_digit_all_off(void);
void fnd_digit_on(uint8_t pin);
void display_setting_mode(uint8_t sec, uint8_t min);
void display_running_mode(uint8_t sec, uint8_t min);

#endif /* FND_H_ */
