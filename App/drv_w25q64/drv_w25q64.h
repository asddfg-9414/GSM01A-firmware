#ifndef __HY_FLASH_H__
#define __HY_FLASH_H__

#include "stm32f4xx.h"                  // Device header

#define STM32_FLASH_BASE        0x08000000  // STM32 Flash 起始地址
//#define	FLASH_USER_START_ADDR	0x00000000	//	写入地址

/* 片选引脚控制宏 */
#define FLASH_CS_L()  HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_RESET)
#define FLASH_CS_H()  HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET)

/* ---------- 基础指令 ---------- */
#define W25Q64_CMD_WRITE_ENABLE     0x06    /* 写使能：允许后续擦除/编程 */
#define W25Q64_CMD_WRITE_DISABLE    0x04    /* 写失能：关闭写操作权限   */
#define W25Q64_CMD_READ_STATUS1     0x05    /* 读状态寄存器1（忙标志位）*/
#define W25Q64_CMD_WRITE_STATUS1    0x01    /* 写状态寄存器1（配置保护）*/

/* ---------- 读数据 ---------- */
#define W25Q64_CMD_READ_DATA        0x03    /* 普通读取，最高 50 MHz     */
#define W25Q64_CMD_FAST_READ        0x0B    /* 快速读取，需 1 字节 Dummy */

/* ---------- 编程 ---------- */
#define W25Q64_CMD_PAGE_PROGRAM     0x02    /* 页编程，最大 256 字节     */

/* ---------- 擦除 ---------- */
#define W25Q64_CMD_SECTOR_ERASE     0x20    /* 4 KB 扇区擦除             */
#define W25Q64_CMD_BLOCK_ERASE_64   0xD8    /* 64 KB 块擦除              */
#define W25Q64_CMD_CHIP_ERASE       0xC7    /* 整片擦除（耗时较长）      */

/* ---------- 器件 ID ---------- */
#define W25Q64_CMD_READ_JEDEC_ID    0x9F    /* 读 JEDEC ID：制造商+类型+容量 */

/* ---------- 复位 ---------- */
#define W25Q64_CMD_ENABLE_RESET     0x66    /* 使能复位（需先发）        */
#define W25Q64_CMD_RESET_DEVICE     0x99    /* 复位芯片（紧跟上条指令）  */

uint8_t w25q64_read_sr1(void);
void w25q64_read(uint32_t addr, uint8_t *buf, uint32_t len);
void w25q64_write(uint32_t addr, const uint8_t *data, uint16_t len);
void w25q64_erase(uint32_t addr);
#endif
