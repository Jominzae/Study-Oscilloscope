#if 1
#include "ds1302.h"

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

#else
/*
 * ds1302.c
 *
 *  Created on: Dec 17, 2025
 *      Author: user
 */

#include "ds1302.h"


void ds1302_main(void);

t_ds1302 ds_time;

//          10진수를       bcd로 변환
// 예) 24 ( 00011000) --> 0010 0100
//STM32의 RTC에서 날짜와 시각정보를 읽어 오는 함수를 작성
uint8_t dec2bcd(uint8_t byte)
{
	unsigned char high, low;


	high = (byte / 10) << 4;
    low  = byte % 10;

	return (high+low);
}


uint8_t bcd2dec(uint8_t byte)
{
	uint8_t high, low;

	low = byte & 0x0f;
	high = (byte >> 4) * 10;

	return (high+low);
}
void ds1302_gpio_init(void)
{
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, 0);
	HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 0);
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 0);
}

void ds1302_dataline_input(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = IO_DS1302_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(IO_DS1302_GPIO_Port, &GPIO_InitStruct);
}

void ds1302_dataline_output(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = IO_DS1302_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(IO_DS1302_GPIO_Port, &GPIO_InitStruct);
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
void ds1302_write(uint8_t command, uint8_t data)
{
	// 1. CE high
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 1);
	// 2. command 전송
	ds1302_tx(command);
	// 3. data 전송
	ds1302_tx(dec2bcd(data));     // SIKWON_1217
	// 4. CE low
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 0);
}

void ds1302_clock(void)
{
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, 1);
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, 0);
}

void ds1302_tx(uint8_t tx)
{
	// 1. ds1302 IO를 output mode로 설정
	ds1302_dataline_output();
	// 2. tx data를 LSB부터 1bit씩 전송
	// 초를 write한다고 가정 하자
	// 0x80
	//  M      L
	//  10000000   0x80
	//& 00000001   0번 bit가 0인지 check 한닥 하자
	//----------------
	//  00000000  이경우 HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 0)
    //  10000000    0x80
	//& 10000000
	//--------------
	//  10000000  7번이  1이라는것을 확인
	//            HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 1)
	for (int i=0; i < 8; i++)  // for (int i=0; i <= 7; i++)
	{
		if (tx & (1 << i))  // 1이상이면
			HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 1);
		else HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, 0);
	}
	// 3. CLK을 High --> LOW
	ds1302_clock();
}
//
// 76543210 : DS1302로부터 LSB 부터 bit가 넘어 온다
void ds1302_rx(uint8_t *data8)
{
	uint8_t temp=0;

	ds1302_dataline_input();

	// DS1302로 부터 LSB부터 1bit씩 받아 들인다.
	for (int i=0; i < 8; i++)
	{
		// 1. 1bit를 읽어 들인다.
		if (HAL_GPIO_ReadPin(IO_DS1302_GPIO_Port, IO_DS1302_Pin))
			temp |= 1 << i;  // 1 인 경우만 set
		// 2. temp 저장
		// 3. clock 전송
		if (i != 7 )
			ds1302_clock();
	}
	*data8 = temp;
}

uint8_t ds1302_read(uint8_t cmd)
{
	uint8_t temp=0;

	// 1. CE high
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 1);
	// 2. cmd 전송
	ds1302_tx(cmd+1);   // cmd+1: read 명령
	// 3. data를 읽어 들이고
	ds1302_rx(&temp);
	// 4. CE low
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, 0);
	// 5. bcd를 dec로 변환 해서 return
	return bcd2dec(temp);
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
#if 1
	ds_time.year=25;
	ds_time.month=12;
	ds_time.date=17;
	ds_time.dayofweek=4;  // wed
	ds_time.hours=16;
	ds_time.minutes=20;
	ds_time.seconds=00;
#endif
	ds1302_gpio_init();
	ds1302_init_time_date();
printf("ds1302_init_time_date !!!\n");
	while(1)
	{
		// 1. ds1302 시간을 읽고
		ds1302_read_time();
printf("ds1302_read_time");
		// 2. ds1302 날짜를 읽고
		ds1302_read_date();
printf("ds1302_read_date");
		// 3. 시간과 날짜를 printf
		printf("***%4d-%2d-%2d %2d:%2d:%2d",
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
#endif
