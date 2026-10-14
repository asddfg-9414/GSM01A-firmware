#ifndef __HY_AD7190_H__
#define __HY_AD7190_H__

#include "main.h"
#include <rtthread.h>

extern SPI_HandleTypeDef hspi3;
extern uint16_t it_count;

// 函数声明
void hy_ad7190_reset(void);
void hy_ad7190_init(void);
void hy_ad7190_led_test(void);
void hy_ad7190_read_reg(void);
void hy_ad7190_power(uint8_t en);
uint32_t hy_ad7190_get_val(void);
unsigned long long get_tare_value(void);
void sampling_rate(uint8_t rate);

#endif

