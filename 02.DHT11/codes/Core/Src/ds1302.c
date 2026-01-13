#include "ds1302.h"
#include <string.h>
#include <stdlib.h>

#define QUEUE_SIZE 4
#define COMMAND_LENGTH 80

extern volatile int rear;
extern volatile int front;
extern volatile uint8_t rx_buff[QUEUE_SIZE][COMMAND_LENGTH];

t_ds1302 ds_time;


unsigned char bcd2dec(unsigned char byte);
unsigned char dec2bcd(unsigned char byte);

void ds1302_gpio_init(void);
void ds1302_init_time_date(void);
void ds1302_write(uint8_t addr, uint8_t data);
void ds1302_DataLine_Input(void);
void ds1302_DataLine_Output(void);
void ds1302_clock(void);
void ds1302_tx(uint8_t tx);
void ds1302_rx(uint8_t *data8);
uint8_t ds1302_read(uint8_t addr);
void ds1302_read_time(void);
void ds1302_read_date(void);
void ds1302_main(void);

void pc_command_processing(void)
{
    if (front != rear)
    {
        char *cmd = (char *)rx_buff[front];

        printf("RX Command: %s\n", cmd);

        if (strncmp(cmd, "setrtc", 6) == 0 && strlen(cmd) >= 18)
        {
            char temp[3];
            temp[2] = 0;
            strncpy(temp, cmd + 6, 2);
            ds_time.year = (uint8_t)atoi(temp);

			strncpy(temp, cmd + 8, 2);
			ds_time.month = (uint8_t)atoi(temp);

			strncpy(temp, cmd + 10, 2);
			ds_time.date = (uint8_t)atoi(temp);

			strncpy(temp, cmd + 12, 2);
			ds_time.hours = (uint8_t)atoi(temp);

			strncpy(temp, cmd + 14, 2);
			ds_time.minutes = (uint8_t)atoi(temp);

			strncpy(temp, cmd + 16, 2);
			ds_time.seconds = (uint8_t)atoi(temp);
            ds1302_init_time_date();
        }
        front = (front + 1) % QUEUE_SIZE;
    }
}

unsigned char bcd2dec(unsigned char byte)
{
	uint8_t high, low;

	low = byte & 0x0f;
	high = (byte >> 4) * 10;

	return (high+low);
}

//          10진수를       bcd로 변환
// 예) 24 ( 00011000) --> 0010 0100
//STM32의 RTC에서 날짜와 시각정보를 읽어 오는 함수를 작성
unsigned char dec2bcd(unsigned char byte)
{
	unsigned char high, low;


	high = (byte / 10) << 4;
    low  = byte % 10;

	return (high+low);
}


void ds1302_gpio_init(void)
{
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, 0);
	HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 0);
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 0);
}

void ds1302_init_time_date(void)
{
	ds1302_write(ADDR_SECONDS, ds_time.seconds);
	ds1302_write(ADDR_MINUTES, ds_time.minutes);
	ds1302_write(ADDR_HOURS, ds_time.hours);
	ds1302_write(ADDR_DATE, ds_time.date);
	ds1302_write(ADDR_MONTH, ds_time.month);
	ds1302_write(ADDR_DAYOFWEEK, ds_time.dayofweek);
	ds1302_write(ADDR_YEAR, ds_time.year);
}

void ds1302_write(uint8_t addr, uint8_t data)
{
	// 1. CE High
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 1);
	// 2. addr 전송
	ds1302_tx(addr);
	// 3. data 전송
	ds1302_tx(dec2bcd(data));     // SIKWON_1217
	// 4. CE low
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 0);
}

void ds1302_DataLine_Input(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pin : PH0 */
  GPIO_InitStruct.Pin = IO_DS1302_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;			//Change Output to Input
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(IO_DS1302_GPIO_Port, &GPIO_InitStruct);

	return;
}


void ds1302_DataLine_Output(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/*Configure GPIO pin : PH0 */
  GPIO_InitStruct.Pin = IO_DS1302_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;			//Change Input to Output
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;   // SIKWON LOW : 2MHZ HIGH: 25MHZ~100MHZ
  HAL_GPIO_Init(IO_DS1302_GPIO_Port, &GPIO_InitStruct);

	return;
}

void ds1302_clock(void)
{
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, 1);
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, 0);
}
void ds1302_tx(uint8_t tx)
{
	ds1302_DataLine_Output();
	// 초를 write
	//       M      L
	// 80h   10000000
	//     & 00000001  0번 bit가 0인지 1인지를 체크한다고 하자
	//    =============
	//       00000000
	// 80h   10000000
	//     & 10000000  7번 bit가 0인지 1인지를 체크한다고 하자
	//    =============
	//       10000000  7번은 1이다.
	for (int i=0; i < 8; i++)
	{
		if (tx & (1 << i))   // 1이상이면
		{
			// bit가 set상태
			HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 1);
		}
		else  // bit가 reset상태
		{
			HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 0);
		}
		ds1302_clock();
	}
}
// data=0
// 76543210  : LSB부터 DS1302에서  bit가 넘어 온다.
// HAL_GPIO_ReadPin bit가 0이면  data의 변수의 해당 bit를 0
//                        1                          1
void ds1302_rx(uint8_t *data8)
{
	uint8_t temp = 0;

	ds1302_DataLine_Input();   // 입력 모드로 전환

	// LSB부터 DS1302에서 넘어 온다.
	for (int i=0; i < 8; i++)
	{
		// 1. bit를 읽고
		if (HAL_GPIO_ReadPin(IO_DS1302_GPIO_Port, IO_DS1302_Pin))
		{
			// 2.
			temp |= 1 << i;
		}
		if (i != 7)
			ds1302_clock();
	}

	*data8 = temp;
}

uint8_t ds1302_read(uint8_t addr)
{
	uint8_t data8bits = 0;

	// 1. CE High
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 1);
	// 2. addr 전송
	ds1302_tx(addr+1);
	// 3. data 읽어 들인다.
	ds1302_rx(&data8bits);
	// 4. CE low
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 0);

	return bcd2dec(data8bits);
}

void ds1302_read_time(void)
{
	ds_time.seconds = ds1302_read(ADDR_SECONDS);
	ds_time.minutes = ds1302_read(ADDR_MINUTES);
	ds_time.hours = ds1302_read(ADDR_HOURS);
}

void ds1302_read_date(void)
{
	ds_time.date = ds1302_read(ADDR_DATE);
	ds_time.month = ds1302_read(ADDR_MONTH);
	ds_time.dayofweek = ds1302_read(ADDR_DAYOFWEEK);
	ds_time.year = ds1302_read(ADDR_YEAR);
}

void ds1302_main(void)
{
	// 초기화
#if 1
	ds_time.year=25;
	ds_time.month=12;
	ds_time.date=17;
	ds_time.dayofweek=4;
	ds_time.hours=17;
	ds_time.minutes=40;
	ds_time.seconds=00;
#endif
	ds1302_gpio_init();   // CLK IO CE을 LOW로 만든다.
	// 시간을 setting
    ds1302_init_time_date();

	while (1)
	{
		pc_command_processing();
		// 1. ds1302 시간을 읽고
		ds1302_read_time();
		// 2. ds1302 날짜를 읽고
		ds1302_read_date();
		// 3. 시간과 날짜를 printf
		printf("***%4d-%2d-%2d %2d:%2d:%2d\n",
				ds_time.year+2000,
				ds_time.month,
				ds_time.date,
				ds_time.hours,
				ds_time.minutes,
				ds_time.seconds);
		// 4. 1초 delay
		HAL_Delay(1000);
	}
}
