/**
* @file    pwm.c
* @brief   16비트 타이머3 및 하드웨어 PWM 기반 DC 모터 제어 드라이버
* @date    2026-06-18
* @author  kccistc
*/

#include "pwm.h"

/**
* @brief  타이머3번을 고속(Fast) PWM 모드로 초기화하여 PE5(OC3C) 핀으로 모터 속도 신호 출력
* * [타이머3 PWM 스펙 정보]
* - 모드 : 8비트 고속 PWM (Fast PWM, 상한선 255 고정)
* - 출력 핀 : PE5 (OC3C) 핀 전용 활용
* - 분주비 : 64분주 적용
* - 펄스 타이밍 계산 :
* 16,000,000Hz 메인 클록을 64로 나누면 250,000Hz 속도로 시계가 감아짐.
* 카운터 숫자가 1씩 올라갈 때마다 정확히 4마이크로초가 소모됨.
* 0부터 255까지 총 256개의 숫자를 모두 세고 리셋되는 데 걸리는 전체 주기 시간은
* 4마이크로초 곱하기 256 = 1,024마이크로초 (약 1밀리초, 주파수로 환산하면 약 976Hz)
*/
void init_timer_3_pwm(void)
{
	/* 1. PWM 신호가 방출될 물리 핀 출력 설정 */
	/* PE3(OC3A) 및 PE5(OC3C, 모터 제어 핀)를 출력으로 설정 */
	/* 주의: PE4는 초음파 센서 에코 인터럽트(INT4) 핀이므로 손대지 않고 보존 */
	DDRE |= (1 << PORTE3) | (1 << PORTE5);
	
	/* 2. Waveform Generation Mode 설정 : 8비트 고속 PWM 모드 선택 */
	/* WGM30 비트와 WGM32 비트를 조합하여 고속 PWM 모드를 완성합니다. */
	TCCR3A |= (1 << WGM30);
	TCCR3B |= (1 << WGM32);
	
	/* 3. Compare Match Output Mode 설정 : 채널 C 비반전(Non-inverting) 모드 */
	/* 카운터가 0일 때 5V 출력을 시작하고, OCR3C 값과 일치하면 0V로 떨어뜨리는 모드 */
	TCCR3A |= (1 << COM3C1);
	
	/* 4. Clock Select 설정 : 64 분주비 기어 작동 시작 */
	/* CS31 스위치와 CS30 스위치를 동시에 1로 켜서 타이머 구동을 시작합니다. */
	TCCR3B |= (1 << CS31) | (1 << CS30);
	
	/* 5. 듀티 사이클 초기값 설정 */
	/* 정지 상태(Duty Cycle 0퍼센트)로 안전하게 시작합니다. */
	OCR3C = 0;
}

/**
* @brief  DC 모터 드라이버의 정회전/역회전 방향 제어용 디지털 GPIO 핀 초기화
* * [핀 매핑 정보]
* - PF6 (IN1) : 모터 드라이버 방향 제어 입력 1
* - PF7 (IN2) : 모터 드라이버 방향 제어 입력 2
* - 초기 제어 상태 : PF6을 High로, PF7을 Low로 설정하여 기본 '정회전' 상태로 대기
*/
void init_motor_driver(void)
{
	/* PORTF의 6번, 7번 핀을 방향 제어용 출력 모드로 설정 */
	DDRF |= (1 << PORTF6) | (1 << PORTF7);
	
	/* 안전을 위해 두 핀을 모두 0V로 깨끗하게 초기화(리셋) */
	PORTF &= ~((1 << PORTF6) | (1 << PORTF7));
	
	/* 기본 구동 방향을 '정회전' 모드로 설정 (IN1 = High, IN2 = Low) */
	PORTF |= (1 << PORTF6);
}

// 모터 속도 설정 함수 (0 ~ 255)
void motor_set_speed(int speed)
{
	if (speed > 250) speed = 250;
	else if (speed < 0) speed = 0;
	
	OCR3C = speed;
}
