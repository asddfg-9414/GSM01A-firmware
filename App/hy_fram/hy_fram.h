#ifndef __HY_FRAM_H__
#define __HY_FRAM_H__

#include "main.h"
#include "spi.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include "usart.h"
#include <stdio.h>

extern char buf[60];

/* ---------- 1. 指令码 ---------- */
#define FRAM_WREN     0x06   // 写使能
#define FRAM_WRDI     0x04   // 写禁止
#define FRAM_RDSR     0x05   // 读状态寄存器
#define FRAM_WRSR     0x01   // 写状态寄存器
#define FRAM_READ     0x03   // 读存储器
#define FRAM_WRITE    0x02   // 写存储器
#define FRAM_SLEEP    0xB9   // 进入休眠
#define FRAM_WAKE     0xAB   // 唤醒（任意 CS 低后发送）

/* 片选引脚控制宏 */
#define FRAM_CS_L()  HAL_GPIO_WritePin(FRAM_CS_GPIO_Port, FRAM_CS_Pin, GPIO_PIN_RESET)
#define FRAM_CS_H()  HAL_GPIO_WritePin(FRAM_CS_GPIO_Port, FRAM_CS_Pin, GPIO_PIN_SET)

#define BLK_SZ          (300*2)
#define GROUP_SZ        20
#define TOTAL_GROUPS    100
#define GROUP_DATA_ADDR(g)  ((g)*12*1024)	//每一组数据存放在3个页里面,依次对齐

typedef struct {
    int16_t buf[300];   // 循环缓冲
    uint16_t wr_idx;     // 下一次写位置
    uint16_t cnt;        // 当前已存个数（0-299）
	uint8_t cur_slot;   // 当前正在填充的槽(0-19),20表示当前组填满
	uint8_t cur_group;	// 当前组号(0-99)
	uint8_t modified;	// 当前组内数据有过更新
} slot_t;

extern slot_t ram;

void fram_spi_write(uint32_t addr, uint8_t *data, uint16_t len);
void fram_spi_read(uint32_t addr, uint8_t *data, uint16_t len);
void fram_test(void);

void push_sample(int16_t v);
void user_switch_group(uint8_t new_g);
void user_switch_slot(void);
void user_delete_slot(uint8_t slot_idx);
void user_delete_group(void);
void fram_init(void);

#endif

