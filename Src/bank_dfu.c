#include "stm32f1xx_hal.h"

#include "evse_dfu.h"
#include "system.h"
#include "delay.h"


#define INTER_FLASH_PAGE_SIZE 2048
#define APP_NAME "0:/app_upgrade_pkg.bin"
#define BL_NAME "0:/bl_upgrade_pkg.bin"

static uint8_t buf[INTER_FLASH_PAGE_SIZE];
static struct head package_head;

static struct head *app_head = (struct head *)APP_HEAD_ADDR ;
static struct head *bl_head = (struct head *)BL_HEAD_ADDR ;


static bool flash_page_program(uint32_t page_addr, uint8_t *buf, uint16_t len)
{
	uint16_t pos = 0;
	uint32_t PageError = 0;
    FLASH_EraseInitTypeDef f;
    f.TypeErase = FLASH_TYPEERASE_PAGES;
    f.PageAddress = page_addr;
    f.NbPages = 1;
	HAL_FLASH_Unlock(); 
	while(pos < len)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, page_addr + pos, *(uint32_t *)&buf[pos]) == HAL_OK){
			pos += 4;
		}
		__NOP();__NOP();
	}
	HAL_FLASH_Lock();
	return true;

}
 static bool inter_flash_page_write(uint32_t page_addr, uint8_t *buf, uint16_t len)
{
	uint16_t pos = 0;
	while(pos < len)
	{
		if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, page_addr + pos, *(uint32_t *)&buf[pos]) == HAL_OK){
			pos += 4;
		}
		__NOP();__NOP();
	}
	
	return true;
}

static uint8_t gen_crc(uint8_t *str, uint32_t len)
{
	uint8_t crc = 0;
	uint32_t i;
	for(i = 0; i < len; i ++)
	{
		crc ^= str[i];
	}
	return crc;
}

static bool upgrade_pack_check()
{
	uint32_t read_len, i = 0, pos;
	struct head *phead = NULL;
	memcpy(&package_head, (void *)UPDATE_HEAD_ADDR, sizeof(package_head));
	if(package_head.magic != 'W'){
		goto invalid;
	}

	if(package_head.type == APP ){
		phead = app_head;
	}else if(package_head.type == BL){
		phead = bl_head;
	}else{
		goto invalid;
	}
	
	if(package_head.addr < 0x08000000){
		goto invalid;
	}
	
	if(memcmp(package_head.crc, phead->crc, sizeof(package_head.crc)) == 0){
		HAL_GPIO_WritePin(GPIOE, LED_RED_Pin|LED_GREEN_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOE, LED_GREEN_Pin, GPIO_PIN_SET);
		memset(buf, 0x00, 2048);
		flash_page_program(UPDATE_HEAD_ADDR, buf, 2048);
		return false;
	}
	if(((1 << get_pcb_version()) & package_head.depends) == 0){
		goto invalid;
	}

	for(pos = 0 ; pos < package_head.len; )
	{
		read_len = (pos + INTER_FLASH_PAGE_SIZE) <= package_head.len ? INTER_FLASH_PAGE_SIZE : package_head.len - pos;
		memcpy(buf, (void *)(BANK_ADDR + pos), read_len);
		if(gen_crc(buf, read_len) != package_head.crc[i++])
			goto invalid;
		pos += read_len;
	}

	return true;
invalid:
	HAL_GPIO_WritePin(GPIOE, LED_RED_Pin|LED_GREEN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, LED_RED_Pin, GPIO_PIN_SET);
	return false;
}

static bool program( struct head *phead)
{
	uint16_t pack_cnt, i;

	pack_cnt = ((phead->len + 2047) >> 11);
	if(phead->type == BL)
	{
		phead->addr = BANK_ADDR;
		phead->reserve[0] = 'M';
	}
	else
	{
	    FLASH_EraseInitTypeDef f;
		uint32_t PageError = 0;
	    f.TypeErase = FLASH_TYPEERASE_PAGES;
	    f.PageAddress = phead->addr;
	    f.NbPages = pack_cnt;
		HAL_FLASH_Unlock(); 
		if(HAL_FLASHEx_Erase(&f, &PageError) != HAL_OK){
			return true;
		}
		for(i = 0; i < pack_cnt ; i ++)
		{
			memcpy(buf, BANK_ADDR  + i * INTER_FLASH_PAGE_SIZE, INTER_FLASH_PAGE_SIZE);
			inter_flash_page_write(phead->addr + i * INTER_FLASH_PAGE_SIZE, buf, INTER_FLASH_PAGE_SIZE);
		}
		HAL_FLASH_Lock(); 
	}
	return true;
}

static bool verify(struct head *phead)
{
	uint16_t pack_cnt, i;

	pack_cnt = phead->len >> 11;
	for(i = 0; i < pack_cnt ; i ++)
	{
		if(gen_crc((uint8_t *)(phead->addr + i * 2048), 2048) == phead->crc[i])
		{
			continue;
		}
		return false;
	}
	if( phead->len & 0x7FF)
	{
		if(gen_crc((uint8_t *)(phead->addr + i * 2048), phead->len & 0x7FF) != phead->crc[i])
		{
			return false;
		}

	}
	return true;
}
 void write_head(struct head *phead)
{
	uint32_t PageError = 0;
    FLASH_EraseInitTypeDef f;

	memcpy(&buf, (void *)HEAD_BASE_ADDR, 2048);
	if(phead->type == APP)
		memcpy(buf, phead, sizeof(struct head));
	else
		memcpy(&buf[1024], phead, sizeof(struct head));


    f.TypeErase = FLASH_TYPEERASE_PAGES;
    f.PageAddress = HEAD_BASE_ADDR;
    f.NbPages = 1;
	HAL_FLASH_Unlock(); 

	if(HAL_FLASHEx_Erase(&f, &PageError) != HAL_OK){
		goto exit;
	}
	inter_flash_page_write(HEAD_BASE_ADDR, buf, 2048);
exit:
	HAL_FLASH_Lock();
}

 bool bank_upgrade_proc()
{
	if(upgrade_pack_check())
	{
		HAL_GPIO_WritePin(GPIOE, LED_RED_Pin|LED_GREEN_Pin, GPIO_PIN_SET);
		if(program( &package_head))
		{
			if(verify(&package_head))
			{
				write_head(&package_head);
				memset(buf, 0x00, 2048);
				flash_page_program(UPDATE_HEAD_ADDR, buf, 2048);
				HAL_GPIO_WritePin(GPIOE, LED_RED_Pin|LED_GREEN_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(GPIOE, LED_GREEN_Pin, GPIO_PIN_SET);
				return true;
			}
		}else{
			HAL_GPIO_WritePin(GPIOE, LED_RED_Pin|LED_GREEN_Pin, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOE, LED_RED_Pin, GPIO_PIN_SET);
			return false;

		}
		return true;
	}

	return false;
}


