#ifndef __HY_HMI_H_
#define __HY_HMI_H_

#include "stm32f4xx.h"                  // Device header

extern uint8_t tx_real;

void hy_tx_hmi(void);
void calculate_shift(float real);
void interval_conversion(float real);
void hy_pedal_press(void);
void receive_data_parse(void);
void hmi_res(void);

#endif
