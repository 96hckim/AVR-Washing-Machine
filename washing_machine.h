/*
* washing_machine.h
*
* Created: 2026-06-19 오전 9:47:25
*  Author: kccistc
*/


#ifndef WASHING_MACHINE_H_
#define WASHING_MACHINE_H_

#include <stdint.h>

typedef enum {
	STATE_STANDBY,  // 대기 모드 (초기 상태)
	STATE_SETTING,  // 시간 설정 모드
	STATE_RUNNING   // 수행 모드
} system_state_t;

typedef enum {
	WASH = 0,       // 세탁 단계
	RINSE,          // 헹굼 단계
	SPIN            // 탈수 단계
} washing_type_t;

typedef struct {
	uint16_t ms;    // 밀리초 카운트
	uint8_t sec;    // 초 데이터
	uint8_t min;    // 분 데이터
} washing_time_t;

typedef struct {
	washing_type_t type;       // 세탁 모드 종류
	washing_time_t set_time;   // 설정된 기본 시간
	uint8_t motor_speed;       // 해당 단계의 모터 PWM 속도
} washing_machine_t;

#endif /* WASHING_MACHINE_H_ */