/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_host.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <rtthread.h>
#include "bsp_board.h"
#include "drv_tmc5240.h"
#include "drv_ad7190.h"
#include "svc_export.h"
#include "alg_kalman.h"
#include "svc_hmi.h"
#include "drv_fram.h"
#include "drv_w25q64.h"
#include <math.h>

#include "usb_host.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#include "usbh_core.h"
#include "drv_w25q64.h"
// 声明 USB 主机句柄
extern USBH_HandleTypeDef hUsbHostFS;
// 声明应用程序状态变量
extern ApplicationTypeDef Appli_state;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define THREAD_PRIORITY         10
#define THREAD_STACK_SIZE       1024
#define THREAD_TIMESLICE        5

static rt_thread_t task1_tid = RT_NULL;
static rt_thread_t save_data_ing_tid = RT_NULL;
static rt_thread_t hmi_uart_tid = RT_NULL;
static rt_thread_t get_data_tid = RT_NULL;
static rt_thread_t pedal_tid = RT_NULL;
static rt_sem_t dynamic_sem_get = RT_NULL;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
extern uint8_t RxBuffer[1];
extern char buf[60];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
//void receive_data_parse(void);
//void export_data(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern float x1;
extern int32_t x2;
extern uint8_t x3;
extern float max_val;
uint32_t zero;

uint8_t time_1s = 0;
static void task1(void *parameter)
{
    while (1)
    {
		receive_data_parse();
		if(time_1s++ >= 99)
		{
			time_1s = 0;
			MX_USB_HOST_Process();
			if((Appli_state == APPLICATION_DISCONNECT) || (Appli_state == APPLICATION_IDLE))
			{
				flash_access = 0;
			}
			else
			{
				flash_access = 1;
			}
		}
		rt_thread_mdelay(10);
	}
}

static void hmi_uart(void *parameter)
{
    while (1)
    {
		svc_hmi_tx();
		rt_thread_mdelay(10);
	}
}

// 分段参数（根据你的最新数据重新计算）
#define X0      0.0f      // 实测空载原始读数
#define X100    1096.4f      // 实测空载原始读数
#define X200    219.6f   // 200g 原始读数

#define X500    551200   // 500g 原始读数,50
#define X1000   1098270  // 1000g 原始读数
#define X2000   2210000  // 2000g 原始读数


static void get_data(void *parameter)
{
	static rt_err_t result;
    while (1)
    {
				result = rt_sem_take(dynamic_sem_get, RT_WAITING_FOREVER);
				if(result == RT_EOK)
				{
						x1 = round(x2 / 109.635f); 
						x1 /= 10;		//	此时x1单位为g
					
						if(x1 > max_val)
						{
								max_val = x1;
						}

						calculate_shift(max_val);	//计算当前挡位
						interval_conversion(x1);	//计算曲线图所需数值
						push_sample(x1);	//当前槽放入最新数据
				}
				tx_real = 1;	//发送最新数值
	}
}

static void save_data_ing(void *parameter)
{
	while(1)
	{
		if(export_start == 1)
		{
			while(Appli_state != APPLICATION_READY)
			{
					MX_USB_HOST_Process();
			}		
			//挂载文件系统
			Mount_FatFs();

			FIL	file;
			FRESULT res = Create_File(&file);
			//打开/创建文件成功
			if(res == FR_OK)
			{
				//字符串必须有换行符"\n"
				TCHAR strn[]="\n";  
				while(export_start == 1)
				{
					if(receive_data_1 == 1)
					{
						receive_data_1 = 0;
						
						f_puts(save_data, &file); 
						f_puts(strn, &file);
						
						if(Save_Data() == FR_OK)
						{
							sprintf(buf,"click get_list,0\xff\xff\xff");		//获取下一个序号
							HAL_UART_Transmit(&huart1,(uint8_t *)buf,strlen(buf),2000);
						}
					}
					rt_thread_mdelay(5);
				}
				//使用完毕关闭文件
				f_close(&file);
			}
			else
			{
				//printf("Open file error，error code: %d\r\n", res);
			}					
		}	
		rt_thread_mdelay(100);
	}
}

void pedal_thread_entry(void *parameter)
{
		while(1)
		{
				svc_hmi_pedal_scan();
				rt_thread_mdelay(10);
		}
}

/*定义自己的存储设备*/
/*用户存储设备扇区字节数*/
#define User_Sector 512
/*用户存储设备FatFs对象*/
#define User_FatFs USBHFatFS
/*用户存储设备卷路径*/
#define User_SDPath USBHPath

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_SPI1_Init();
  MX_SPI3_Init();
  MX_FATFS_Init();
  MX_USB_HOST_Init();
  MX_TIM2_Init();
  MX_SPI2_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
		HAL_UART_Receive_IT(&huart1,RxBuffer,1);
		HAL_GPIO_WritePin(HMI_EN_GPIO_Port,HMI_EN_Pin,GPIO_PIN_RESET);
	
		drv_ad7190_reset();
		drv_ad7190_init();
		alg_kalman_init();
		rt_thread_mdelay(200);
		drv_ad7190_power(1);
		rt_thread_mdelay(200);

		fram_init();
		hmi_res();

		dynamic_sem_get = rt_sem_create("dsem1", 0, RT_IPC_FLAG_PRIO);
		/* 创建线程 1 */
		task1_tid = rt_thread_create("task_1",
																	task1, (void*)1,
																	THREAD_STACK_SIZE,
																	4, THREAD_TIMESLICE);

			if (task1_tid != RT_NULL)
					rt_thread_startup(task1_tid);
	
						/* 创建线程 2 */
		hmi_uart_tid = rt_thread_create("task_2",
																	hmi_uart, (void*)1,
																	THREAD_STACK_SIZE,
																	THREAD_PRIORITY, THREAD_TIMESLICE);

			if (hmi_uart_tid != RT_NULL)
					rt_thread_startup(hmi_uart_tid);
			
				/* 创建线程 3 */
		get_data_tid = rt_thread_create("task_3",
																	get_data, (void*)1,
																	1024,
																	THREAD_PRIORITY, THREAD_TIMESLICE);

			if (get_data_tid != RT_NULL)
					rt_thread_startup(get_data_tid);

			
		/* 创建线程 4 */
		save_data_ing_tid = rt_thread_create("task_4",
																	save_data_ing, (void*)1,
																	2048,
																	5, THREAD_TIMESLICE);

			if (save_data_ing_tid != RT_NULL)
					rt_thread_startup(save_data_ing_tid);
			

			/* 创建线程 5 */
    pedal_tid = rt_thread_create("pedal",
                                pedal_thread_entry, (void*)1,
                                THREAD_STACK_SIZE,
                                5, THREAD_TIMESLICE);

    if (pedal_tid != RT_NULL)
        rt_thread_startup(pedal_tid);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    MX_USB_HOST_Process();

    /* USER CODE BEGIN 3 */
		rt_thread_mdelay(500);
		HAL_IWDG_Refresh(&hiwdg);	//喂狗
		HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int32_t v_data;
int32_t temp_val;
int32_t mean_val;

uint16_t count1 = 0;


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
		if(htim->Instance == TIM2)		
		{		
				v_data = drv_ad7190_get_val() - zero;
					
				if(v_data < 1096*10 && temp_val == 0)	//一轮数据获取中没有超过10g需要进行滤波
				{		
						mean_val += v_data;
				}
				else if(v_data > temp_val)		//一轮数据获取中有一次超过10g就取最大值
				{
						temp_val = v_data;
				}
				count1++;
				if(count1 >= it_count)		//根据定时器频率计算读取次数(100ms)
				{
						count1 = 0;
						if(temp_val == 0)	//一轮数据获取中没有超过10g需要进行滤波
						{
								temp_val = mean_val/it_count;		//小于10g取平均值
						}
						else if(temp_val > 1096*5000)			//大于上限值舍去
						{
								temp_val = max_val;
						}
						
						x2 = temp_val;
						temp_val = 0;
						mean_val = 0;
              
						rt_sem_release(dynamic_sem_get);	//数值换算
				}
		}

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
