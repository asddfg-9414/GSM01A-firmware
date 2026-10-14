#ifndef __HY_TMC5240_H__
#define __HY_TMC5240_H__

#include "main.h"
#include "stm32f4xx_hal.h"
#include <rtthread.h>

extern SPI_HandleTypeDef hspi1;

// ===== TMC5240 register set =====

#define TMC5240_GCONF          0x00  // 全局配置寄存器，用于设置芯片的全局功能和模式
#define TMC5240_GSTAT          0x01  // 全局状态寄存器，用于读取芯片的状态信息
#define TMC5240_IFCNT          0x02  // 中断计数寄存器，用于读取中断事件的计数
#define TMC5240_SLAVECONF      0x03  // 从设备配置寄存器，用于设置从设备的配置信息
#define TMC5240_INP_OUT        0x04  // 输入输出寄存器，用于读取或设置输入输出引脚的状态
#define TMC5240_X_COMPARE      0x05  // 比较寄存器，用于设置比较值
#define TMC5240_OTP_PROG       0x06  // 一次性可编程寄存器，用于编程一次性可编程存储器
#define TMC5240_DRV_CONF       0x0A  // 驱动配置寄存器，用于设置驱动器的配置参数
#define TMC5240_GLOBAL_SCALER  0x0B  // 全局缩放寄存器，用于设置全局缩放因子

#define TMC5240_IHOLD_IRUN     0x10  // 保持电流和运行电流寄存器，用于设置保持电流和运行电流的值
#define TMC5240_TPOWERDOWN     0x11  // 功率下降时间寄存器，用于设置功率下降时间
#define TMC5240_TSTEP          0x12  // 步进时间寄存器，用于设置步进时间
#define TMC5240_TPWMTHRS       0x13  // PWM阈值寄存器，用于设置PWM阈值
#define TMC5240_TCOOLTHRS      0x14  // 冷却阈值寄存器，用于设置冷却阈值
#define TMC5240_THIGH          0x15  // 

#define TMC5240_RAMPMODE       0x20  // 加减速模式寄存器，用于设置加减速模式
#define TMC5240_XACTUAL        0x21  // 实际位置寄存器，用于读取实际位置值
#define TMC5240_VACTUAL        0x22  // 实际速度寄存器，用于读取实际速度值
#define TMC5240_VSTART         0x23  // 启动速度寄存器，用于设置启动速度
#define TMC5240_A1             0x24  // 第一阶段加速度寄存器，用于设置第一阶段加速度
#define TMC5240_V1             0x25  // 第一阶段速度寄存器，用于设置第一阶段速度
#define TMC5240_AMAX           0x26  // 最大加速度寄存器，用于设置最大加速度
#define TMC5240_VMAX           0x27  // 最大速度寄存器，用于设置最大速度
#define TMC5240_DMAX           0x28  // 最大减速度寄存器，用于设置最大减速度
#define TMC5240_TVMAX          0x29  // 最大速度时间寄存器，用于设置最大速度时间
#define TMC5240_D1             0x2A  // 第一阶段减速度寄存器，用于设置第一阶段减速度
#define TMC5240_VSTOP          0x2B  // 停止速度寄存器，用于设置停止速度
#define TMC5240_TZEROWAIT      0x2C  // 零位等待时间寄存器，用于设置零位等待时间
#define TMC5240_XTARGET        0x2D  // 目标位置寄存器，用于设置目标位置
#define TMC5240_V2             0x2E  // 第二阶段速度寄存器，用于设置第二阶段速度
#define TMC5240_A2             0x2F  // 第二阶段加速度寄存器，用于设置第二阶段加速度

#define TMC5240_D2             0x30  // 第二阶段减速度寄存器，用于设置第二阶段减速度
#define TMC5240_AACTUAL        0x31  // 实际加速度寄存器，用于读取实际加速度值
#define TMC5240_VDCMIN         0x33  // 最小直流电压寄存器，用于设置最小直流电压
#define TMC5240_SWMODE         0x34  // 软件模式寄存器，用于设置软件模式
#define TMC5240_RAMPSTAT       0x35  // 加减速状态寄存器，用于读取加减速状态
#define TMC5240_XLATCH         0x36  // 锁存位置寄存器，用于读取锁存位置值
#define TMC5240_ENCMODE        0x38  // 编码器模式寄存器，用于设置编码器模式
#define TMC5240_XENC           0x39  // 编码器位置寄存器，用于读取编码器位置值
#define TMC5240_ENC_CONST      0x3A  // 编码器常数寄存器，用于设置编码器常数
#define TMC5240_ENC_STATUS     0x3B  // 编码器状态寄存器，用于读取编码器状态
#define TMC5240_ENC_LATCH      0x3C  // 编码器锁存寄存器，用于读取编码器锁存值
#define TMC5240_ENC_DEVIATION  0x3D  // 编码器偏差寄存器，用于读取编码器偏差值

#define TMC5240_ADC_VSUPPLY_AIN 0x50 // 电源电压和模拟输入寄存器，用于读取电源电压和模拟输入值
#define TMC5240_ADC_TEMP        0x51 // 温度寄存器，用于读取温度值
#define TMC5240_OTW_OV_VTH      0x52 // 过温过压阈值寄存器，用于设置过温过压阈值

#define TMC5240_MSLUT0         0x60  // 微步查找表0寄存器，用于设置微步查找表0
#define TMC5240_MSLUT1         0x61  // 微步查找表1寄存器，用于设置微步查找表1
#define TMC5240_MSLUT2         0x62  // 微步查找表2寄存器，用于设置微步查找表2
#define TMC5240_MSLUT3         0x63  // 微步查找表3寄存器，用于设置微步查找表3
#define TMC5240_MSLUT4         0x64  // 微步查找表4寄存器，用于设置微步查找表4
#define TMC5240_MSLUT5         0x65  // 微步查找表5寄存器，用于设置微步查找表5
#define TMC5240_MSLUT6         0x66  // 微步查找表6寄存器，用于设置微步查找表6
#define TMC5240_MSLUT7         0x67  // 微步查找表7寄存器，用于设置微步查找表7
#define TMC5240_MSLUTSEL       0x68  // 微步查找表选择寄存器，用于选择微步查找表
#define TMC5240_MSLUTSTART     0x69  // 微步查找表起始寄存器，用于设置微步查找表起始值
#define TMC5240_MSCNT          0x6A  // 微步计数寄存器，用于读取微步计数值
#define TMC5240_MSCURACT       0x6B  // 微步电流实际寄存器，用于读取微步电流实际值
#define TMC5240_CHOPCONF       0x6C  // 斩波配置寄存器，用于设置斩波器的配置参数，如斩波频率、斩波模式等
#define TMC5240_COOLCONF       0x6D  // 冷却配置寄存器，用于设置冷却功能的配置参数，如冷却电流、冷却时间等
#define TMC5240_DCCTRL         0x6E  // 直流控制寄存器，用于设置直流控制参数，如直流电压、直流电流等
#define TMC5240_DRVSTATUS      0x6F  // 驱动状态寄存器，用于读取驱动器的状态信息，如过流、过热等

#define TMC5240_PWMCONF        0x70  // PWM配置寄存器，用于设置PWM的配置参数，如PWM频率、PWM模式等
#define TMC5240_PWMSCALE       0x71  // PWM缩放寄存器，用于设置PWM的缩放因子
#define TMC5240_PWM_AUTO       0x72  // PWM自动寄存器，用于设置PWM的自动模式参数
#define TMC5240_SG4_THRS       0x74  // SG4阈值寄存器，用于设置SG4的阈值
#define TMC5240_SG4_RESULT     0x75  // SG4结果寄存器，用于读取SG4的结果
#define TMC5240_SG4_IND        0x76  // SG4指示寄存器，用于指示SG4的状态



extern void drv_tmc5240_init(void);
extern void tmc_test(void);
extern void __tmc_spi_writeint(uint8_t reg, uint32_t dat);
extern uint32_t __tmc_spi_readint(uint8_t reg);


#endif

