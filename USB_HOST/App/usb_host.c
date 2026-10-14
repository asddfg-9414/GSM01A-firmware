/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file            : usb_host.c
  * @version         : v1.0_Cube
  * @brief           : This file implements the USB Host
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

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"

/* USER CODE BEGIN Includes */
#include "bsp_board.h"

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Host core handle declaration */
USBH_HandleTypeDef hUsbHostFS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * user callback declaration
 */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
void MX_USB_HOST_Init(void)
{
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

  /* USER CODE END USB_HOST_Init_PreTreatment */

  /* Init host Library, add supported class and start the library. */
  if (USBH_Init(&hUsbHostFS, USBH_UserProcess, HOST_FS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_RegisterClass(&hUsbHostFS, USBH_MSC_CLASS) != USBH_OK)
  {
    Error_Handler();
  }
  if (USBH_Start(&hUsbHostFS) != USBH_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */

  /* USER CODE END USB_HOST_Init_PostTreatment */
}

/*
 * Background task
 */
void MX_USB_HOST_Process(void)
{
  /* USB Host Background task */
  USBH_Process(&hUsbHostFS);
}
/*
 * user callback definition
 */
static void USBH_UserProcess  (USBH_HandleTypeDef *phost, uint8_t id)
{
  /* USER CODE BEGIN CALL_BACK_1 */
  switch(id)
  {
    /* �� USB ������Ҫѡ���豸������ʱ���������¼� */
    case HOST_USER_SELECT_CONFIGURATION:
      // ��ǰδʵ���κ��߼���ֱ������
      break;

    /* �� USB �豸�������Ͽ�����ʱ���������¼� */
    case HOST_USER_DISCONNECTION:
      // ��Ӧ�ó���״̬����Ϊ�豸�ѶϿ�
      Appli_state = APPLICATION_DISCONNECT;
      // �ر� LED �ƣ����� bsp_board_led1(0) ��ʾ�ر� LED��
      bsp_board_led1(0);
      break;

    /* �� USB �豸�����������򱻼���ʱ���������¼� */
    case HOST_USER_CLASS_ACTIVE:
      // ��Ӧ�ó���״̬����Ϊ�豸��׼����
      Appli_state = APPLICATION_READY;
      break;

    /* �� USB �豸���ӵ�����ʱ���������¼� */
    case HOST_USER_CONNECTION:
      // ��Ӧ�ó���״̬����Ϊ�豸������
      Appli_state = APPLICATION_START;
      // �� LED �ƣ����� bsp_board_led1(1) ��ʾ�� LED��
      bsp_board_led1(1);
      break;

    /* Ĭ�����������δ֪�¼� */
    default:
      // δ֪�¼��������κδ���
      break;
  }
  /* USER CODE END CALL_BACK_1 */
}

/**
  * @}
  */

/**
  * @}
  */

