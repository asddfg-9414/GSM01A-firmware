#include "svc_hmi.h"
#include "usart.h"
#include "svc_export.h"
#include "tim.h"
#include "alg_kalman.h"
#include "drv_ad7190.h"
#include "drv_fram.h"
#include <math.h>

uint8_t RxBuffer[1];
uint8_t g_usart_rx_buf[30];
uint16_t g_usart_rx_sta = 0;
char buf[60];

uint8_t key1,key2;

float x1;
int32_t x2;
uint8_t x3;
float max_val;

uint8_t shift = 1;		//当前挡位
uint8_t last_shift = 1;	//上次挡位
uint8_t tx_real = 0;	//100ms置1,上传当前值

extern uint32_t zero;
extern int32_t v_data;
extern int32_t temp_val;
extern int32_t mean_val;

void export_data(void);
void addt(void);

void svc_hmi_tx(void)	//10ms进一次
{
	if(last_shift != shift)	//挡位切换或者串口屏需要恢复曲线图
	{
		last_shift = shift;
		sprintf(buf,"main.shift.val=%d\xff\xff\xff", shift);	//发送当前挡位
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
		sprintf(buf,"cle s0.id,0\xff\xff\xff");				//清空曲线图
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
		
		addt();
	}
	
	if(tx_real)	//100ms进一次
	{
		tx_real = 0;
		sprintf(buf,"data.r_peak.val=%d\xff\xff\xff",(uint32_t)(max_val*10));	//发送最高值
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
		sprintf(buf,"data.r_h0.val=%d\xff\xff\xff",(uint8_t)((max_val+750)/7500*150));	//发送最高值进度条数据
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
		sprintf(buf,"data.r_real.val=%d\xff\xff\xff",(int32_t)(x1*10));	//发送当前值
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);						
		sprintf(buf,"data.r_h1.val=%d\xff\xff\xff",(uint8_t)((x1+750)/7500*150));			//发送当前值进度条数据
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
		sprintf(buf,"data.flash_access.val=%d\xff\xff\xff",flash_access);		//发送U盘是否插入标志位
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);

		sprintf(buf,"add s0.id,0,%d\xff\xff\xff",x3);		//发送曲线图数据
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
	}
	
	if(key1==1)
	{
		key1=0;
		sprintf(buf,"click B_LOG,0\xff\xff\xff");		//发送按下保存键
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
	}
	else if(key2==1)
	{
		key2=0;
		sprintf(buf,"click B_ZERO,0\xff\xff\xff");		//发送按下清零键
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
	}
}

//串口屏重启
void hmi_res(void)
{
	sprintf(buf,"x01\xff\xff\xff");
	HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
	sprintf(buf,"rest\xff\xff\xff");
	HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
}

void svc_hmi_pedal_scan(void)
{
		static uint8_t press_key1 = 0;
		static uint8_t press_key2 = 0;
		static uint8_t count1 = 0;
		static uint8_t count2 = 0;
		if(HAL_GPIO_ReadPin(L_INT_GPIO_Port, L_INT_Pin) == GPIO_PIN_SET)
		{
				if(press_key1 == 0)
				{
						if(++count1 >= 5)
						{
								press_key1 = 1;
						}
				}
				else if(press_key1 == 1)
				{
						count1 = 0;
				}
		}
		else
		{	
				if(press_key1 == 0)
				{
						count1 = 0;
				}
				else if(press_key1 == 1)
				{
						if(++count1 >=5)
						{
								key1 = 1;
								press_key1 = 0;
						}
				}	
		}
		
		if(HAL_GPIO_ReadPin(R_INT_GPIO_Port, R_INT_Pin) == GPIO_PIN_SET)
		{
				if(press_key2 == 0)
				{
						if(++count2 >= 5)
						{
								press_key2 = 1;
						}
				}
				else if(press_key2 == 1)
				{
						count2 = 0;
				}
		}
		else
		{		
				if(press_key2 == 0)
				{
						count2 = 0;
				}
				else if(press_key2 == 1)
				{
						if(++count2 >=5)
						{
								key2 = 1;
								press_key2 = 0;
						}
				}	
		}
}

void calculate_shift(float real)	//计算当前挡位
{
	/* 区间上限表，升序排列 */
	static const float limit[] = {10, 50, 100, 500, 1000, 3000};
	static const int   shift_map[] = {1, 2, 3, 4, 5, 6, 7}; /* 最后 7 对应“以上” */
	
	real = fabs(real);
    for (int i = 0; i < 6; ++i)
	{
        if (real < limit[i])
		{
			shift = shift_map[i];
			return;
		}
	}
	
    shift = shift_map[6];   /* >= 3000 */
}

void interval_conversion(float real)	//计算曲线图当前值
{	
	switch(shift)
	{
		case 1: if(real < -1.5f)
				{
					real = -1.5f;
//					x1 = -1.5f;
				}
				x3 = (uint8_t)((real+1.5f)/15*150);
				break;
				
		case 2:	if(real < -7.5f)
				{
						real = -7.5f;
//						x1 = -7.5f;
				}
				x3 = (uint8_t)((real+7.5f)/75*150);
				break;
				
		case 3: if(real < -15)
				{
						real = -15;
//						x1 = -15;
				}
				x3 = (uint8_t)((real+15)/150*150);
				break;
				
		case 4: if(real < -75)
				{
						real = -75;
//						x1 = -75;
				}
				x3 = (uint8_t)((real+75)/750*150);
				break;
				
		case 5: if(real < -150)
				{
						real = -150;
//						x1 = -150;
				}
				x3 = (uint8_t)((real+150)/1500*150);
				break;
				
		case 6: if(real < -450)
				{
						real = -450;
//						x1 = -450;
				}
				x3 = (uint8_t)((real+450)/4500*150);
				break;
				
		case 7: if(real < -750)
				{
						real = -750;
//						x1 = -750;
				}
				x3 = (uint8_t)((real+750)/7500*150);
				break;
	}
}

//曲线图挡位切换后向串口屏透传数据
void addt(void)
{
	sprintf(buf,"addt s0.id,0,%d\xff\xff\xff", ram.cnt);
	HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
	rt_thread_mdelay(80);	//等待串口屏响应
		
	for(uint16_t i=0; i<ram.cnt; i++)
	{
		if(ram.cnt < 300)
		{
			interval_conversion(ram.buf[i]);	//计算曲线图所需数值
		}
		else
		{
			uint16_t ri = (ram.wr_idx + i) % 300;
			interval_conversion(ram.buf[ri]);
		}
		sprintf(buf,"%c", x3);
		HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
	}
	
	//确保透传结束，以免影响下一条指令
	sprintf(buf,"\x01\xff\xff\xff");
	HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
}

void receive_data_parse(void)
{
	uint32_t r_temp = 0;
	
	if((g_usart_rx_sta & 0xE000) == 0xE000)
	{
		if(g_usart_rx_buf[0] == 0x0A)		//清零/修改频率
		{
			if(g_usart_rx_buf[1] == 0)
			{
				HAL_TIM_Base_Stop_IT(&htim2);
				__HAL_TIM_SET_COUNTER(&htim2, 0);
				__HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
				x1 = 0;
				x2 = 0;
				x3 = 15;
				v_data = 0;
				temp_val = 0;
				mean_val = 0;
				max_val = 0;
				alg_kalman_init();
				
				zero = get_tare_value();   //100Hz采样
				sampling_rate(g_usart_rx_buf[2]);
				rt_thread_mdelay(50);
				
				ram.wr_idx     = 0;
				ram.cnt        = 0;
				
				sprintf(buf,"click p3,0\xff\xff\xff");
				HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
				
				HAL_TIM_Base_Start_IT(&htim2);
			}
			else if(g_usart_rx_buf[1] == 1)
			{
				last_shift = 0;		//刷新串口屏曲线图显示
			}
		}
		else if(g_usart_rx_buf[0] == 0x0B)
		{
			switch(g_usart_rx_buf[1])
			{
				case 0: x1 = 0; x2 = 0; x3 = 15; v_data = 0;
						temp_val = 0; mean_val = 0; max_val = 0;
						user_switch_slot();		//切换到下一槽数据
						break;
				
				case 1: r_temp = g_usart_rx_buf[2];
						user_delete_slot(r_temp);	//删除单个数据
						break;
				
				case 2: user_delete_group();		//删除整组数据
						break;
				
				case 3: r_temp = g_usart_rx_buf[2]-1;
						ram.cur_slot = g_usart_rx_buf[3];
						user_switch_group(r_temp);	//切换新的组
						break;
				
				case 4:	fram_init();			//清空所有数据
						break;
			}
		}
		else if(g_usart_rx_buf[0] == 0x0C)		//数据导出操作
		{
			export_data();
		}
		
		g_usart_rx_sta = 0;
		memset(g_usart_rx_buf, 0, sizeof(g_usart_rx_buf));
	}
}

void export_data(void)
{
	switch(g_usart_rx_buf[1])
	{
		case 0: export_start = 1;
				memset(file_name, 0, sizeof(file_name));		//先清零，保证文件名以 0 结尾
				memcpy(file_name, &g_usart_rx_buf[2], 18);		//固定 18 字节文件名（缓冲已放大到 24）
				sprintf(buf,"click get_list,0\xff\xff\xff");
				HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
				break;
		
		case 1: export_start = 0;
				HAL_TIM_Base_Start_IT(&htim2);
				break;
		
		case 2:	memset(save_data, 0, sizeof(save_data));		//先清零，保证数据行以 0 结尾
				memcpy(save_data, &g_usart_rx_buf[2], 28);		//固定 28 字节表格行（缓冲已放大到 32）
				receive_data_1 = 1;
				break;
				
		default :	HAL_TIM_Base_Stop_IT(&htim2);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
		if(huart->Instance == USART1)
		{
				if(g_usart_rx_sta & 0x8000)
				{
						if(g_usart_rx_sta & 0x4000)
						{
								if(RxBuffer[0] == 0xFF)
									g_usart_rx_sta += 0x1000; //如果接收到3个0xFF，g_usart_rx_sta&0FFF为0xE000表示接收完成
								else
										g_usart_rx_sta = 0;
						}
						else
						{
								//判断是否是包尾
								if(RxBuffer[0] == 0xFF)
								{
										g_usart_rx_sta |= 0x4000;
								}
								else
								{
										//越界保护：接收缓冲仅 30 字节，超长帧直接丢弃并复位状态机
										if((uint32_t)(g_usart_rx_sta & 0x0FFF) < (uint32_t)sizeof(g_usart_rx_buf))
										{
												g_usart_rx_buf[g_usart_rx_sta & 0x0FFF] = RxBuffer[0];
												g_usart_rx_sta++;
										}
										else
										{
												g_usart_rx_sta = 0;
										}
								}
						}
				}
				else 
				{
						if(g_usart_rx_buf[0] == 0xAA)
						{
								//接收到第二个包头
								if(RxBuffer[0] == 0x55)
								{
										//开始接收数据标志
										g_usart_rx_sta = 0x8000;
								}
								else
								{
										memset(g_usart_rx_buf, 0, sizeof(g_usart_rx_buf));
								}
						}
						else if(RxBuffer[0] == 0xAA)
						{
								//接收到包头
								g_usart_rx_buf[0] = 0xAA;
						}
				}
				
			HAL_UART_Receive_IT(&huart1, RxBuffer, 1);
		}
}
