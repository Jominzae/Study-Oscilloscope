/*
 * ds1302.h
 *
 *  Created on: Dec 17, 2025
 *      Author: User
 */

#ifndef INC_DS1302_H_
#define INC_DS1302_H_

#include "main.h"
// write command 만 define 하고 read 시에는 write + 1 것을 함.
#define ADDR_SECONDS 0x80	// write
#define ADDR_MINUTES 0x82	// write
#define ADDR_HOURS	 0x84	//
#define ADDR_DATE	 0x86
#define ADDR_MONTH	 0x88
#define ADDR_DAYOFWEEK 0x8A
#define ADDR_YEAR	 0x8C
#define ADDR_WRITEPROTECTED 0x8E

#endif /* INC_DS1302_H_ */
