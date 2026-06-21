/**
* @file    uart0.h
* @brief   ATmega128A UART0 직렬 통신 드라이버 인터페이스
* @date    2026-06-16
* @author  kccistc
*/

#ifndef UART0_H_
#define UART0_H_

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>

#define UART0_BAUD_9600         9600
#define UART0_BAUD_115200       115200

#define UART0_DEFAULT_BAUD      UART0_BAUD_9600

void init_uart(void);
int transmit_uart(char data, FILE *stream);
void process_uart_command(void);

#endif /* UART0_H_ */
