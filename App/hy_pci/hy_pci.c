#include "hy_pci.h"

/**
 * @brief 控制输出1的状态
 * @param state: 输出状态，1表示高电平，0表示低电平
 */
void hy_pci_out1(uint8_t state)
{
    if(state == 1)
    {
        // 设置输出1为高电平
        HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_SET);
    }
    else
    {
        // 设置输出1为低电平
        HAL_GPIO_WritePin(OUT1_GPIO_Port, OUT1_Pin, GPIO_PIN_RESET);
    }
}

/**
 * @brief 控制输出2的状态
 * @param state: 输出状态，1表示高电平，0表示低电平
 */
void hy_pci_out2(uint8_t state)
{
    if(state == 1)
    {
        // 设置输出2为高电平
        HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_SET);
    }
    else
    {
        // 设置输出2为低电平
        HAL_GPIO_WritePin(OUT2_GPIO_Port, OUT2_Pin, GPIO_PIN_RESET);
    }
}

void hy_pci_led1(uint8_t state)
{
    if(state == 1)
    {
        // 点亮LED1
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
    }
    else
    {
        // 熄灭LED1
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
    }
}

void hy_pci_led2(uint8_t state)
{
    if(state == 1)
    {
        // 点亮LED2
        HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
    }
    else
    {
        // 熄灭LED2
        HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);
    }
}

