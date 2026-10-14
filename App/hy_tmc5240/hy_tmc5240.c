#include "hy_tmc5240.h"

void __tmc_spi_cs(uint8_t en)
{
    if (en == 1)
    {
        HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, GPIO_PIN_RESET);
    }
    else
    {
        HAL_GPIO_WritePin(SPI1_NSS_GPIO_Port, SPI1_NSS_Pin, GPIO_PIN_SET);
    }
}

uint8_t __tmc_read_write(uint8_t byte)
{
    uint8_t receivedByte = 0;

    // 发送数据并接收响应
    HAL_SPI_TransmitReceive(&hspi1, &byte, &receivedByte, 1, HAL_MAX_DELAY);

    return receivedByte;
}

void __tmc_spi_writeint(uint8_t reg, uint32_t dat)
{
    uint8_t dat1, dat2, dat3, dat4;

    dat1 = dat & 0xFF;         // 获取低8位
    dat2 = (dat >> 8) & 0xFF;  // 获取次低8位
    dat3 = (dat >> 16) & 0xFF; // 获取次高8位
    dat4 = (dat >> 24) & 0xFF; // 获取高8位

    // 使能电机对应的CS引脚
    __tmc_spi_cs(1);

    // 发送寄存器地址和写命令（0x80）
    __tmc_read_write(reg | 0x80);

    // 发送数据的高24位
    __tmc_read_write(dat4);
    __tmc_read_write(dat3);
    __tmc_read_write(dat2);

    // 发送数据的低8位
    __tmc_read_write(dat1);

    // 取消电机对应的CS引脚
    __tmc_spi_cs(0);
}

uint32_t __tmc_spi_readint(uint8_t reg)
{
    uint8_t dat1, dat2, dat3, dat4;
    uint8_t i;

    uint32_t dat = 0;
    // 使能电机对应的CS引脚
    __tmc_spi_cs(1);

    // 发送寄存器地址
    __tmc_read_write(reg);

    // 发送4个虚拟字节以使能数据读取（根据TMC5240的数据手册）
    __tmc_read_write(0x00);
    __tmc_read_write(0x00);
    __tmc_read_write(0x00);
    __tmc_read_write(0x00);
    // 取消电机对应的CS引脚
    __tmc_spi_cs(0);

    for (i = 0; i<5; i++)
    {
        // 延时，等待电机准备好发送数据
    }
    // 使能电机对应的CS引脚
    __tmc_spi_cs(1);
    __tmc_read_write(reg);
    // 读取数据的高24位
    dat4 = __tmc_read_write(0x00);
    dat3 = __tmc_read_write(0x00);
    dat2 = __tmc_read_write(0x00);

    // 读取数据的低8位
    dat1 = __tmc_read_write(0x00);
    // 取消选中电机对应的CS引脚
    __tmc_spi_cs(0);
    // 重新组合接收到的数据
    dat = (dat4 << 24) + (dat3 << 16) + (dat2 << 8) + dat1;

    return dat;
}

void hy_tmc5240_init(void)
{
	__tmc_spi_writeint(TMC5240_GSTAT, 0x00000007); // 清除全局状态寄存器的错误标志和警告标志，确保系统正常运行
	__tmc_spi_writeint(TMC5240_GCONF, 0x00000004); // 设置全局配置寄存器为默认值，关闭所有特殊功能，确保系统处于初始状态

	__tmc_spi_writeint(TMC5240_CHOPCONF, 0x00410153); // 配置斩波器参数，包括斩波频率、斩波模式等，以优化电机的运行性能
	__tmc_spi_writeint(TMC5240_PWMCONF, 0xC44C261E); // 配置PWM参数，包括PWM频率、PWM模式等，以实现精确的速度控制
	__tmc_spi_writeint(TMC5240_TPWMTHRS, 7735); // 设置静音模式最大值
	
	__tmc_spi_writeint(TMC5240_DRV_CONF, 0x00000020); // 配置驱动器参数，包括驱动电流、驱动模式等，以确保电机的稳定运行
	__tmc_spi_writeint(TMC5240_GLOBAL_SCALER, 0); // 设置全局缩放因子为0，表示不进行缩放，直接使用原始值
	__tmc_spi_writeint(TMC5240_IHOLD_IRUN, 0x00030901); // 设置保持电流为7mA，运行电流为3mA，空闲电流为3mA，以平衡电机的运行和能耗
	__tmc_spi_writeint(TMC5240_TPOWERDOWN, 0x0000000A); // 设置功率下降时间为10ms，以在电机停止后快速降低功耗

	__tmc_spi_writeint(TMC5240_XACTUAL, 0); // 设置实际位置为0，表示电机的当前位置
	__tmc_spi_writeint(TMC5240_XTARGET, 0); // 设置目标位置为0，表示电机的目标位置
	__tmc_spi_writeint(TMC5240_VSTART, 10); // 设置启动速度为5，表示电机启动时的速度
	__tmc_spi_writeint(TMC5240_A1, 54717); // 设置第一阶段加速度为1000，表示电机在第一阶段的加速度
	__tmc_spi_writeint(TMC5240_V1, 0); // 设置第一阶段速度为0，表示电机在第一阶段的最终速度

	__tmc_spi_writeint(TMC5240_DMAX, 154717); // 设置最大减速度为153600，表示电机的最大减速度
	__tmc_spi_writeint(TMC5240_D1, 54717); // 设置第一阶段减速度为1400，表示电机在第一阶段的减速度
	__tmc_spi_writeint(TMC5240_VSTOP, 10); // 设置停止速度为10，表示电机停止时的速度
	__tmc_spi_writeint(TMC5240_TZEROWAIT, 1000); // 设置零位等待时间为1000，表示电机在达到零位时的等待时间
	__tmc_spi_writeint(TMC5240_AMAX, 54717); // 设置最大加速度为5000，表示电机的最大加速度
	
	__tmc_spi_writeint(TMC5240_RAMPMODE, 	0); //定位模式,使用全部add
	
	unsigned int pwmconf = __tmc_spi_readint(TMC5240_PWMCONF);	
	unsigned int chopconf = __tmc_spi_readint(TMC5240_CHOPCONF);

}

void tmc_test(void)
{
	__tmc_spi_writeint(TMC5240_XTARGET, 	1547170);  //一圈脉冲数51,200  0.3mm
	__tmc_spi_writeint(TMC5240_VMAX,      154717);		//1s 3圈 51200*3=153,600‬脉冲154,716
//	rt_thread_mdelay(15000);
//	__tmc_spi_writeint(TMC5240_XTARGET, 	0);
//	rt_thread_mdelay(15000);
}

