#include "hy_flash.h"
#include "stm32f4xx_hal.h"
#include "string.h"
#include <stdio.h>
#include "spi.h"

uint8_t w25q64_read_sr1(void)
{
	uint8_t cmd = 0x05;
	uint8_t sr;
	
	FLASH_CS_L();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);      /* 发命令 + 地址 */
    HAL_SPI_Receive(&hspi2, &sr, 1, HAL_MAX_DELAY);    /* 读数据 */
    FLASH_CS_H();
	
	return sr;
}

void __w25q64_write_enable(void)
{
	uint8_t cmd = W25Q64_CMD_WRITE_ENABLE;
	FLASH_CS_L();
	HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);
	FLASH_CS_H();
}

void w25q64_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint8_t cmd[4] = {
        W25Q64_CMD_READ_DATA,
        (addr>>16)&0xFF,
        (addr>>8)&0xFF,
        addr&0xFF
    };
    FLASH_CS_L();
    HAL_SPI_Transmit(&hspi2, cmd, 4, HAL_MAX_DELAY);      /* 发命令 + 地址 */
    HAL_SPI_Receive(&hspi2, buf, len, HAL_MAX_DELAY);  	  /* 读数据 */
    FLASH_CS_H();
}

void w25q64_write(uint32_t addr, const uint8_t *data, uint16_t len)
{
	uint16_t write_len;
	uint32_t already = 0;
	
	/* 1. 计算要擦除的扇区范围 */
    uint32_t start_sec = addr / 4096;
    uint32_t end_sec   = (addr + len - 1) / 4096;

    /* 2. 逐个扇区擦除 */
    for (uint32_t sec = start_sec; sec <= end_sec; sec++)
	{
        w25q64_erase(sec * 4096);
    }
	
	do
	{	
		write_len = (len > 256) ? 256 : len;
		len -= write_len;
		
		__w25q64_write_enable();
		uint8_t cmd[4] = {
			W25Q64_CMD_PAGE_PROGRAM,
			(addr>>16)&0xFF,
			(addr>>8)&0xFF,
			addr&0xFF
		};
	
		FLASH_CS_L();
		HAL_SPI_Transmit(&hspi2, cmd, 4, HAL_MAX_DELAY);
		HAL_SPI_Transmit(&hspi2, data+already, write_len, HAL_MAX_DELAY);
		FLASH_CS_H();
		
		while(w25q64_read_sr1() & 0x01);
		
		addr += 256;
		already += write_len;
	}while(len);
}

/* 4 KB 扇区擦除 */
void w25q64_erase(uint32_t addr)
{
    __w25q64_write_enable();
    uint8_t cmd[4] = {
        W25Q64_CMD_SECTOR_ERASE,
        (addr>>16)&0xFF,
        (addr>>8)&0xFF,
        addr&0xFF
    };
    FLASH_CS_L();
    HAL_SPI_Transmit(&hspi2, cmd, 4, HAL_MAX_DELAY);
    FLASH_CS_H();
	
	while(w25q64_read_sr1() & 0x01);
}
