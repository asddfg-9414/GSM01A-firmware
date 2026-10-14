#ifndef __HY_HMI_H_
#define __HY_HMI_H_

#include "stm32f4xx.h"                  // Device header

extern uint8_t tx_real;

void svc_hmi_tx(void);
void calculate_shift(float real);
void interval_conversion(float real);
void svc_hmi_pedal_scan(void);
void receive_data_parse(void);
void hmi_res(void);

#endif
