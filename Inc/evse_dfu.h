#ifndef __EVSE_DFU_H__
#define __EVSE_DFU_H__

#include <stdint.h>
#include <stdbool.h>
#define NONE 0x00
#define APP 0x01
#define BL 0x02

#define HEAD_BASE_ADDR 0x0803F800
#define UPDATE_HEAD_ADDR 0x0803F000
#define APP_HEAD_ADDR 0x0803F800
#define BL_HEAD_ADDR 0x0803FC00
#define BANK_ADDR 	   0x0801B000
#define BL_BASE_ADDR 	0x08000000
#define BL_BANK_ADDR 	   0x08037000

struct head{
	uint8_t magic;
	uint8_t type;
	uint16_t depends;
	uint16_t ver;
	uint8_t time[6];
	uint32_t len;
	uint32_t addr;
	uint8_t crc[150];
	uint8_t reserve[30];
} ;

#endif
