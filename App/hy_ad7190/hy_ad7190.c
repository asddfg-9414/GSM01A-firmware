#include "hy_ad7190.h"
#include "hy_kalman.h"
#include "tim.h"
#include "stm32f4xx_hal.h"

uint16_t it_count = 100;

// 定义AD7190的ID、配置寄存器、模式寄存器和电压数据变量
uint8_t ad7190_id = 0;
uint32_t conf_reg = 0;
uint32_t mode_reg = 0;

// 控制AD7190的片选信号（CS）
void __ad7190_spi_cs(uint8_t en)
{
    if (en == 1)
    {
        HAL_GPIO_WritePin(SPI3_NSS_GPIO_Port, SPI3_NSS_Pin, GPIO_PIN_RESET); // 片选信号低电平有效
    }
    else
    {
        HAL_GPIO_WritePin(SPI3_NSS_GPIO_Port, SPI3_NSS_Pin, GPIO_PIN_SET); // 片选信号高电平无效
    }
}

// 向AD7190发送单字节数据
void __ad7190_spi_write(uint8_t byte)
{
    // 发送数据
    HAL_SPI_Transmit(&hspi3, &byte, 1, HAL_MAX_DELAY);
}

// 从AD7190接收单字节数据
uint8_t __ad7190_spi_read_byte(void)
{
    uint8_t receivedByte = 0;
    HAL_SPI_Receive(&hspi3, &receivedByte, 1, HAL_MAX_DELAY);
    return receivedByte;
}

// 从AD7190读取寄存器值
uint32_t __ad7190_read_reg(uint8_t reg)
{
    uint8_t dat1, dat2, dat3;
    uint8_t i;
    uint32_t dat = 0;
    __ad7190_spi_write(reg); // 写入寄存器地址

    for (i = 0; i < 5; i++)
    {
        // 延时，等待数据准备就绪
    }

    dat3 = __ad7190_spi_read_byte(); // 读取最高8位
    dat2 = __ad7190_spi_read_byte(); // 读取次高8位
    dat1 = __ad7190_spi_read_byte(); // 读取最低8位
    dat = (dat3 << 16) + (dat2 << 8) + dat1; // 组合成24位数据
    return dat;
}

// 重置AD7190
void hy_ad7190_reset(void)
{
    __ad7190_spi_cs(1); // 使能片选

    for (uint8_t cnt = 0; cnt < 55; cnt++)
    {
        __ad7190_spi_write(0xFF); // 发送复位指令
    }

    __ad7190_spi_cs(0); // 禁用片选
}

// 初始化AD7190
void hy_ad7190_init(void)
{
    rt_thread_mdelay(100); // 延时100ms
    __ad7190_spi_cs(1); // 使能片选
    __ad7190_spi_write(0x60); // 写入ID寄存器地址
    ad7190_id = __ad7190_spi_read_byte(); // 读取ID
    __ad7190_spi_write(0x10); // 写入配置寄存器地址
    __ad7190_spi_write(0x00); // 写入配置数据（CON23-CON16）
    __ad7190_spi_write(0x02); // 写入配置数据（CON15-CON8）
    __ad7190_spi_write(0x07); // 写入配置数据（CON7-CON0），设置增益
    //0x00:GAIN=1;
    //0x03:GAIN=8;
    //0x04:GAIN=16;
    //0x07:GAIN=128; (GAIN=128时，输入范围为+-39.06mV)
    __ad7190_spi_cs(0); // 禁用片选
    rt_thread_mdelay(100); // 延时100ms
    __ad7190_spi_cs(1); // 使能片选
    __ad7190_spi_write(0x08); // 写入模式寄存器地址
    __ad7190_spi_write(0x08); // 写入模式数据（MR23-MR16）
    __ad7190_spi_write(0x00); // 写入模式数据（MR15-MR8）
    __ad7190_spi_write(0x04); // 写入模式数据（MR7-MR0），设置采样率
    //fs(MR0-MR9) = 0x00ff = 255,即数据输出速率为4920/255=19.2sps
    //0x00 0xff：19.2sps
    //0x00 0x3f：78sps
    //0x00 0x1f：164sps
    //0x00 0x0f：328sps
    //0x00 0x05：984sps
    //0x00 0x04：1230sps
    //0x00 0x03：1640sps
    //0x00 0x02：2460sps
    //0x00 0x01：4920sps
    __ad7190_spi_cs(0); // 禁用片选
}

// 获取20个数据并计算平均值作为去皮值
unsigned long long get_tare_value(void)
{
    unsigned long long sum = 0;
    unsigned int value;
    int i;

    // 使能片选
    __ad7190_spi_cs(1);

    // 写入模式寄存器地址
    __ad7190_spi_write(0x08);

    // 写入模式数据
    __ad7190_spi_write(0x08); // MR23-MR16
    __ad7190_spi_write(0x00); // MR15-MR8
    __ad7190_spi_write(0x1f); // MR7-MR0，设置采样率为164sps

    // 禁用片选
    __ad7190_spi_cs(0);

	for (i = 0; i < 160; i++)
    {
        hy_ad7190_get_val();
		}
    // 获取200个数据
    for (i = 0; i < 200; i++)
    {
        value = hy_ad7190_get_val();
//				value = channels.kalman_X;
				rt_thread_mdelay(10);
        sum += value;
    }
		
    // 计算平均值
    unsigned long long average = sum / 200;
		
    return average;
}

void sampling_rate(uint8_t rate)
{
	rate = 3;	//速率功能已废弃，锁定1000Hz
	
		// 使能片选
    __ad7190_spi_cs(1);

    // 写入模式寄存器地址
    __ad7190_spi_write(0x08);

    // 写入模式数据
    __ad7190_spi_write(0x08); // MR23-MR16
    __ad7190_spi_write(0x00); // MR15-MR8
		switch(rate){
//				case 0: __ad7190_spi_write(0xff); // MR7-MR0，设置采样率,10Hz
//								__HAL_TIM_SET_AUTORELOAD(&htim2, 50000);  //修改重转载值
//								it_count = 2;			//中断读取次数
//								break; 
//				case 1: __ad7190_spi_write(0x1f); // MR7-MR0，设置采样率,100Hz
//								__HAL_TIM_SET_AUTORELOAD(&htim2, 9999);
//								it_count = 10;
//								break; 
				case 2: __ad7190_spi_write(0x0f); 		//300Hz
								__HAL_TIM_SET_AUTORELOAD(&htim2, 3332);
								it_count = 30;
								break;
				case 3: __ad7190_spi_write(0x04); 		//1000Hz
								__HAL_TIM_SET_AUTORELOAD(&htim2, 999);
								it_count = 100;
								break;
//				case 4: __ad7190_spi_write(0x03); 		//1500Hz
//								__HAL_TIM_SET_AUTORELOAD(&htim2, 666);
//								it_count = 150;
//								break;
//				case 5: __ad7190_spi_write(0x02); 		//2000Hz
//								__HAL_TIM_SET_AUTORELOAD(&htim2, 499);
//								it_count = 200;
//								break;
//				case 6: __ad7190_spi_write(0x01); 		//4000Hz
//								__HAL_TIM_SET_AUTORELOAD(&htim2, 249); 
//								it_count = 400;
//								break; 
		}
		// 禁用片选
    __ad7190_spi_cs(0);
}

// 测试AD7190的LED功能
void hy_ad7190_led_test(void)
{
    __ad7190_spi_cs(1); // 使能片选
    __ad7190_spi_write(0x28); // 写入GPIO控制寄存器地址
    __ad7190_spi_write(0x24); // 设置LED状态
    __ad7190_spi_cs(0); // 禁用片选
    rt_thread_mdelay(1000); // 延时1000ms
    __ad7190_spi_cs(1); // 使能片选
    __ad7190_spi_write(0x28); // 写入GPIO控制寄存器地址
    __ad7190_spi_write(0x20); // 改变LED状态
    __ad7190_spi_cs(0); // 禁用片选
    rt_thread_mdelay(1000); // 延时1000ms
}

// 读取AD7190的配置寄存器和模式寄存器
void hy_ad7190_read_reg(void)
{
    rt_thread_mdelay(1000); // 延时1000ms
    __ad7190_spi_cs(1); // 使能片选
    conf_reg = __ad7190_read_reg(0x50); // 读取配置寄存器
    __ad7190_spi_cs(0); // 禁用片选
    rt_thread_mdelay(1000); // 延时1000ms
    __ad7190_spi_cs(1); // 使能片选
    mode_reg = __ad7190_read_reg(0x48); // 读取模式寄存器
    __ad7190_spi_cs(0); // 禁用片选
}

// 控制AD7190的电源
void hy_ad7190_power(uint8_t en)
{
    __ad7190_spi_cs(1); // 使能片选
    __ad7190_spi_write(0x28); // 写入GPIO控制寄存器地址
    __ad7190_spi_write(0x24); // 控制电源状态
    __ad7190_spi_cs(0); // 禁用片选
}

// 从AD7190获取测量值
uint32_t hy_ad7190_get_val(void)
{
    uint32_t val = 0;
    __ad7190_spi_cs(1); // 使能片选
    val = __ad7190_read_reg(0x58); // 读取数据
    __ad7190_spi_cs(0); // 使能片选
	
//		hy_kalman_calc(&channels,val);
	
    return val;
}

