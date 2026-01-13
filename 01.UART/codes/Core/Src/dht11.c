/*
 * dht11.c
 *
 *  Created on: Dec 16, 2025
 *      Author: user
 */

#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void delay_us(unsigned long _us);

void dht11_main(void);

void dht11_dataline_input(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	GPIO_InitStruct.Pin = DHT11_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

void dht11_dataline_output(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	GPIO_InitStruct.Pin = DHT11_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

int wait_readpin(GPIO_PinState _pinstate, uint32_t _timeout) {
	uint32_t us_counter = 0;
	while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == _pinstate) {
		delay_us(2);
		us_counter += 2;
		if (us_counter > _timeout) {
			return -1;
		}
	}
	return 0;
}

void dht11_main(void) {
	enum state_t {
		OK, TIMEOUT, VALUE_ERROR, TRANS_ERROR
	};
	enum state_t state = OK;
	uint32_t us_counter = 0;

	int data[6] = { 0 };

	while (1) {
		for (int i = 0; i < 6; i++)
			data[i] = 0;

		state = OK;

		// ========== step1 request start signal : STM32 ==========
		dht11_dataline_output();
		HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
		HAL_Delay(100);		// 100 ms

		HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
		HAL_Delay(20);		// 20 ms

		HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
		delay_us(30);

		dht11_dataline_input();

		if (wait_readpin(1, 50) < 0) {
			state = TIMEOUT;
		}

		// ========== step2 response start signal from DHT11 ==========
		if (state == OK) {
			if (wait_readpin(0, 100) < 0) {
				state = TIMEOUT;
			}
		}

		if (state == OK) {
			if (wait_readpin(1, 100) < 0) {
				state = TIMEOUT;
			}
		}

		// ==========  ==========
		if (state == OK) {
			int pulse[8] = { 0, };

			for (int i = 0; i < 5; i++) {
				for (int j = 7; j >= 0; j--) {
					if (wait_readpin(0, 70) < 0) {
						state = TIMEOUT;
						i = 5;
						j = -1;
					}

					if (state == OK) {
						if (wait_readpin(1, 90) < 0) {
							state = TIMEOUT;
							i = 5;
							j = -1;
						}

						if (state == OK) {
							if (us_counter < 40)
								pulse[j] = 0;
							else if (us_counter > 40)
								pulse[j] = 1;
						}
					}
				}

				if (state == OK) {
					data[i] = pulse[0] << 0 | pulse[1] << 1 | pulse[2] << 2
							| pulse[3] << 3 | pulse[4] << 4 | pulse[5] << 5
							| pulse[6] << 6 | pulse[7] << 7;
				}
			}

			if (state == OK) {
				if (data[4] != data[0] + data[1] + data[2] + data[3]) {
					state = VALUE_ERROR;
				}
			}

			delay_us(60);
			us_counter = 0;
			while (HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == 0) {
				delay_us(2);
				us_counter += 2;
				if (us_counter > 90) {
					break;
				}
			}
		}

		if (state == OK) {
			printf("[TEMP]: %d.%dC\n", data[2], data[3]);
			printf("[HUMI]: %d.%d%%\n", data[0], data[1]);
		} else //if (state != OK)
		{
			printf("error code: %d\n", state);
			printf("data: 0x%02x 0x%02x 0x%02x 0x%02x\n", data[0], data[1],
					data[2], data[3]);
		}

		HAL_Delay(2000);
	}
}
