#ifndef __HY_PCI_H__
#define __HY_PCI_H__

#include "gpio.h"
#include "stdint.h"

/**
 * @brief 控制输出1的状态
 * @param state: 输出状态，1表示高电平，0表示低电平
 */
void hy_pci_out1(uint8_t state);

/**
 * @brief 控制输出2的状态
 * @param state: 输出状态，1表示高电平，0表示低电平
 */
void hy_pci_out2(uint8_t state);

/**
 * @brief 控制LED1的状态
 * @param state: 控制LED亮灭，1表示亮，0表示灭
 */
void hy_pci_led1(uint8_t state);

/**
 * @brief 控LED2的状态
 * @param state: 控制LED亮灭，1表示亮，0表示灭
 */
void hy_pci_led2(uint8_t state);

#endif

