/**
* @file    uart0.c
* @brief   UART0 초기화(9600bps, RX INT, TX Polling) 및 전송 구현
* @date    2026-06-16
* @author  kccistc
*/

#include "washing_machine.h"
#include "uart0.h"

#define QUEUE_SIZE   10   /* 원형 큐 행(Row) 크기: 저장 가능한 최대 문장 수 */
#define QUEUE_LENGTH 80   /* 원형 큐 열(Col) 크기: 한 문장당 최대 글자 수 */

volatile int rear = 0;    /* ISR이 수신한 문장을 저장할 인덱스 (Write Ptr) */
volatile int front = 0;   /* 메인 루프가 명령어를 처리할 인덱st (Read Ptr) */
volatile unsigned char rx_buff[QUEUE_SIZE][QUEUE_LENGTH];

extern system_state_t current_state;
extern washing_machine_t wash_step;
extern void change_system_state(system_state_t system_state);
extern void change_washing_mode(washing_machine_t* current_step);

/**
* @brief USART0 1바이트 수신 완료 인터럽트 서비스 루틴 (ISR)
* @note  PC가 보낸 데이터를 바이트 단위로 받아 문장(\\n, \\r 기준)으로 조립 후 큐에 저장
*/
ISR(USART0_RX_vect)
{
	uint8_t data = UDR0;  /* 하드웨어 수신 버퍼에서 1바이트 읽기 (UDR0 자동 비워짐) */
	static int i = 0;     /* 현재 조립 중인 문장의 문자 인덱스 */
	
	/* 1. 큐 가득 참(Queue Full) 검사: 가득 찼다면 현재 수신 데이터 버림 */
	if ((rear + 1) % QUEUE_SIZE == front)
	{
		return;
	}
	
	/* 2. 문자 파싱 및 문장 조립 */
	if (data == '\n' || data == '\r')
	{
		rx_buff[rear][i] = '\0';        /* 문장의 끝(NULL) 마감 처리 */
		i = 0;                          /* 다음 문장 조립을 위해 문자 인덱스 초기화 */
		rear = (rear + 1) % QUEUE_SIZE; /* 큐의 rear 전진 (행 이동) */
	}
	else
	{
		/* 버퍼 오버플로우 방지 처리가 포함된 문자 저장 */
		if (i < (QUEUE_LENGTH - 1))
		{
			rx_buff[rear][i++] = data;
		}
	}
}

/**
* @brief  UART0 초기화 함수
* - 속도: 9600bps (2배속 모드 모드 설정)
* - 데이터: 8비트, 패리티 없음, 정지 비트 1비트 (AVR 기본값 사양)
* - 송수신: 송신(TX) 및 수신(RX) 활성화
* - 인터럽트: 수신 완료(RX) 인터럽트 허용
*/
void init_uart(void)
{
	/* 1. 보레이트 레이트 설정: 9600bps (16MHz, U2X0=1 기준 UBRR=207) */
	UBRR0H = 0x00;
	UBRR0L = 207;
	
	/* 2. UCSR0A: 2배속 모드 활성화 (U2X0 = 1) */
	UCSR0A |= 1 << U2X0;
	
	/* 3. UCSR0B: RX/TX 활성화 및 RX 완료 인터럽트 허용
	- RXEN0: 수신기 활성화
	- TXEN0: 송신기 활성화
	- RXCIE0: 수신 완료 인터럽트 켜기 */
	UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;
}

/**
* @brief  UART0로 1바이트를 전송하는 함수 (표준 출력 printf 연동 규격)
* @param  data   : 전송할 8비트 문자 데이터 (printf 엔진이 쪼개서 보내준 한 글자)
* @param  stream : 표준 출력 스트림 포인터 (fdev_setup_stream 연동용, 여기선 내부적으로 자동 처리됨)
* @return int    : 전송 성공 시 0을 반환 (표준 입출력 함수들의 정상 종료 약속)
*/
int transmit_uart(char data, FILE *stream)
{
	/* UDRE0(UART Data Register Empty) 플래그 감시
	- UDR0 송신 버퍼가 비워질 때까지 (1이 될 때까지) while 루프로 대기합니다. */
	while (!(UCSR0A & (1 << UDRE0)));
	
	/* 하드웨어 버퍼에 데이터를 집어넣으면 즉시 송신선(TX)으로 쏴집니다. */
	UDR0 = data;
	
	return 0;
}

void process_uart_command(void)
{
	// Receive Queue 가 비어있으므로 return
	if (front == rear) return;
	
	char *cmd = (char*)rx_buff[front];
	
	printf("UART Rx: %s\n", cmd);
	
	// 시작 명령 처리
	if (strncmp(cmd, "START_WASHING", strlen("START_WASHING")) == 0)
	{
		// 현재 이미 돌고 있는 상태가 아닐 때만 실행
		if (current_state != STATE_RUNNING)
		{
			change_system_state(STATE_RUNNING);
			change_washing_mode(&wash_step);
		}
	}
	// 정지 명령 처리
	else if (strncmp(cmd, "STOP_WASHING", strlen("STOP_WASHING")) == 0)
	{
		// 현재 세탁기가 돌고 있는 상태일 때만 실행
		if (current_state == STATE_RUNNING)
		{
			change_system_state(STATE_STANDBY);
		}
	}
	
	front = (front + 1) % QUEUE_SIZE;
}
