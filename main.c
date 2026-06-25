/*
* 06.DCMOTOR_PWM_CONTROL
*
* Created: 2026-06-15 오전 11:36:39
* Author : kccistc
*/

#define F_CPU 16000000UL

#include "washing_machine.h"
#include "button.h"
#include "led.h"
#include "fnd.h"
#include "uart0.h"
#include "pwm.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>

void init_washing_machine_system(void);
void init_timer_0(void);
void change_system_state(system_state_t system_state);
void change_washing_mode(washing_machine_t* current_step);
void update_current_time(void);
void update_running_step(void);
void on_button_0_press(void);
void on_button_1_press(void);
void on_button_2_press(void);
void on_button_3_press(void);
void print_washing_status(void);

FILE OUTPUT = FDEV_SETUP_STREAM(transmit_uart, NULL, _FDEV_SETUP_WRITE);
volatile uint32_t ms_count = 0;

washing_machine_t wash_step  = { WASH,  { 0, 10, 0 }, 250 };
washing_machine_t rinse_step = { RINSE, { 0, 10, 0 }, 250 };
washing_machine_t spin_step  = { SPIN,  { 0, 10, 0 }, 250 };

system_state_t current_state = STATE_STANDBY;
washing_machine_t *current_mode;
volatile washing_time_t *current_time;
volatile washing_time_t running_clock;

void (*on_button_press_funcs[])(void) = {
	on_button_0_press,
	on_button_1_press,
	on_button_2_press,
	on_button_3_press
};

void (*fnd_mode_funcs[])(uint8_t, uint8_t) = {
	NULL,
	display_setting_mode,
	display_running_mode
};

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 6;
	ms_count++; // 1ms 카운트
}

int main(void)
{
	button_id_t button_id;
	int i;
	uint32_t last_refresh_time = 0;
	
	init_washing_machine_system();
	init_timer_0();
	init_button();
	init_led();
	init_fnd();
	init_uart();
	init_timer_3_pwm();
	init_motor_driver();
	
	stdout = &OUTPUT;
	sei();
	
	while (1)
	{
		// 실시간 입력 버튼 스캔
		for(i = 0; i < BUTTON_COUNT; i++)
		{
			button_id = (button_id_t)i;
			
			if (get_button_state(button_id))
			{
				on_button_press_funcs[button_id]();
				break;
			}
		}
		
		//// 1ms 주기 타이머
		if (ms_count - last_refresh_time >= 1)
		{
			last_refresh_time = ms_count;
			
			if (current_state != STATE_STANDBY)
			{
				if (current_state == STATE_RUNNING)
				{
					update_current_time();
					update_running_step();
				}
				// 현재 상태에 맞는 FND 화면 출력
				fnd_mode_funcs[current_state](current_time->sec, current_time->min);
			}
		}
		
		// UART 명령어 수신 검사
		process_uart_command();
	}
	
	return 0;
}

// 시스템 초기화
void init_washing_machine_system(void)
{
	current_state = STATE_STANDBY;
	current_mode = &wash_step;
	current_time = &current_mode->set_time;
}

void init_timer_0(void)
{
	TCNT0 = 6;
	
	TCCR0 &= ~(1 << CS02 | 1 << CS01 | 1 << CS00);
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00; // 64분주
	
	TIMSK |= 1 << TOIE0; // 오버플로우 인터럽트 활성화
}

void change_system_state(system_state_t system_state)
{
	current_state = system_state;
	
	if (current_state == STATE_STANDBY)
	{
		motor_set_speed(0);
		led_all_off();
		fnd_data_all_off();
		fnd_digit_all_off();
		
		current_mode = &wash_step;
		current_time = &current_mode->set_time;
	}
}

void change_washing_mode(washing_machine_t* current_step)
{
	current_mode = current_step;
	
	if (current_state == STATE_RUNNING)
	{
		running_clock = current_mode->set_time;
		current_time = &running_clock;
		
		// 구동 중일 때만 해당 스텝의 모터 속도 반영
		motor_set_speed(current_step->motor_speed);
	}
	else
	{
		current_time = &current_mode->set_time;
		motor_set_speed(0);
	}
	
	// 설정 및 구동 중에만 해당하는 LED 켜기
	if (current_state != STATE_STANDBY)
	{
		led_on(current_mode->type);
	}
}

void update_current_time(void)
{
	if (current_time->min == 0 && current_time->sec == 0) return;

	current_time->ms++;
	if (current_time->ms >= 1000)
	{
		print_washing_status();
		current_time->ms = 0;
		
		if (current_time->sec == 0)
		{
			if (current_time->min > 0)
			{
				current_time->min--;
				current_time->sec = 59;
			}
		}
		else
		{
			current_time->sec--;
		}
	}
}

void update_running_step(void)
{
	if (current_time->min != 0 || current_time->sec != 0) return;
	
	// 현재 모드 타임아웃 시 다음 스텝으로 이동
	switch (current_mode->type)
	{
		case WASH:
		// 세탁 종료 -> 헹굼 단계로 전환
		printf("[SYSTEM]: WASH Done. Moving to RINSE.\r\n");
		change_washing_mode(&rinse_step);
		break;
		
		case RINSE:
		// 헹굼 종료 -> 탈수 단계로 전환
		printf("[SYSTEM]: RINSE Done. Moving to SPIN.\r\n");
		change_washing_mode(&spin_step);
		break;
		
		case SPIN:
		// 탈수 완료 (모든 단계 완료)
		printf("[SYSTEM]: All Cycles Finished Success!\r\n");
		change_system_state(STATE_STANDBY);
		break;
		
		default:
		break;
	}
}

// 시작/정지
void on_button_0_press(void)
{
	switch (current_state)
	{
		case STATE_STANDBY:
		change_system_state(STATE_RUNNING);
		change_washing_mode(&wash_step);
		break;
		
		case STATE_RUNNING:
		change_system_state(STATE_STANDBY);
		break;
		
		default:
		break;
	}
}

// 설정 모드 변경
void on_button_1_press(void)
{
	switch (current_state)
	{
		case STATE_STANDBY:
		change_system_state(STATE_SETTING);
		change_washing_mode(&wash_step);
		break;
		
		case STATE_SETTING:
		switch (current_mode->type)
		{
			case WASH:
			change_washing_mode(&rinse_step);
			break;
			
			case RINSE:
			change_washing_mode(&spin_step);
			break;
			
			case SPIN:
			change_system_state(STATE_STANDBY);
			break;
			
			default:
			break;
		}
		break;
		
		default:
		break;
	}
}

// 시간 설정(분) 변경
void on_button_2_press(void)
{
	if (current_state != STATE_SETTING) return;

	current_time->min++;
	if (current_time->min >= 60)
	{
		current_time->min = 0;
	}
}

void on_button_3_press(void)
{
}

// UART 시리얼 로그 출력
void print_washing_status(void)
{
	if (current_mode == NULL || current_time == NULL) return;

	const char* mode_name;
	switch (current_mode->type)
	{
		case WASH:
		mode_name = "WASH";
		break;
		
		case RINSE:
		mode_name = "RINSE";
		break;
		
		case SPIN:
		mode_name = "SPIN";
		break;
		
		default:
		mode_name = "UNKNOWN";
		break;
	}

	printf("[MODE]: %s | [TIME]: %02dm %02ds\r\n", mode_name, current_time->min, current_time->sec);
}
