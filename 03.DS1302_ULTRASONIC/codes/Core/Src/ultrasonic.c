/*
 * ultrasonic.c
 *
 *  Created on: 2025. 12. 18.
 *      Author: User
 */

#include "main.h"
#include <stdio.h>

uint32_t distance = 0;		// 거리 측정 펄스값
uint8_t ic_cpt_flag = 0;	// 상승 하강 둘다 만났다는 flag

extern volatile int TIM10_ultrasonic_counter;
extern void delay_us(unsigned long us);
void ultrasonic_processing(void);
void make_trigger_pulse(void);

// echo 펄스의 상승에지 하강에지가 발생시 이곳으로 들어온다.
// 결국 2번 들어 온다./
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	static uint8_t is_first_capture = 0;

	if(htim->Instance == TIM3)
	{
		if(is_first_capture == 0)	// 상승 에지 !!
		{
			__HAL_TIM_SET_COUNTER(htim, 0);
			is_first_capture = 1;	// 상승 에지 flag set
		} else if(is_first_capture == 1)	//하강 에지 !!
		{
			is_first_capture = 0;
			distance = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			ic_cpt_flag = 1;
		}
	}
}

void ultrasonic_processing(void)
{
	if(TIM10_ultrasonic_counter >= 100)
	{
		TIM10_ultrasonic_counter = 0;
		make_trigger_pulse();
		if(ic_cpt_flag == 1)
		{
			ic_cpt_flag = 0;
			distance = distance * 0.034 / 2;	// 1us 0.034cm 이동하는데 소요되는 시간
			printf("dis : %d\n", distance);
		}
	}
}

void make_trigger_pulse(void)
{
	HAL_GPIO_WritePin(TRIG_PIN_GPIO_Port, TRIG_PIN_Pin, 0);
	delay_us(2);
	HAL_GPIO_WritePin(TRIG_PIN_GPIO_Port, TRIG_PIN_Pin, 1);
	delay_us(10);
	HAL_GPIO_WritePin(TRIG_PIN_GPIO_Port, TRIG_PIN_Pin, 0);
}
