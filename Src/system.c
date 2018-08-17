#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "main.h"

uint8_t get_pcb_version()
{
	uint8_t version = 0x00;
	version = (HAL_GPIO_ReadPin(VERSION0_GPIO_Port, VERSION0_Pin) << 3);
	version |= (HAL_GPIO_ReadPin(VERSION1_GPIO_Port, VERSION1_Pin) << 2);
	version |= (HAL_GPIO_ReadPin(VERSION2_GPIO_Port, VERSION2_Pin) << 1);
	version |= (HAL_GPIO_ReadPin(VERSION3_GPIO_Port, VERSION3_Pin) << 0);
	return version;
}

