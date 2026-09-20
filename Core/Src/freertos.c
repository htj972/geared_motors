/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "OLED.h"
#include "tim.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
osThreadId LOGIHandle;
osThreadId OLED_DISPLAYHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartTask_LOGI(void const * argument);
void StartTask_OLED(void const * argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of LOGI */
  osThreadDef(LOGI, StartTask_LOGI, osPriorityNormal, 0, 128);
  LOGIHandle = osThreadCreate(osThread(LOGI), NULL);

  /* definition and creation of OLED_DISPLAY */
  osThreadDef(OLED_DISPLAY, StartTask_OLED, osPriorityIdle, 0, 128);
  OLED_DISPLAYHandle = osThreadCreate(osThread(OLED_DISPLAY), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartTask_LOGI */
/**
  * @brief  Function implementing the LOGI thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTask_LOGI */
void StartTask_LOGI(void const * argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartTask_LOGI */
	HAL_GPIO_WritePin(EN12V_GPIO_Port,EN12V_Pin,GPIO_PIN_SET);
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
		if((HAL_GPIO_ReadPin(UP_GPIO_Port,UP_Pin)==GPIO_PIN_RESET)&&
			(HAL_GPIO_ReadPin(DOWN_GPIO_Port,DOWN_Pin)==GPIO_PIN_SET))
		{
			HAL_GPIO_WritePin(OA_GPIO_Port,OA_Pin,GPIO_PIN_SET);
			HAL_GPIO_WritePin(OB_GPIO_Port,OB_Pin,GPIO_PIN_RESET);
		}
		else if((HAL_GPIO_ReadPin(DOWN_GPIO_Port,DOWN_Pin)==GPIO_PIN_RESET)&&
			(HAL_GPIO_ReadPin(UP_GPIO_Port,UP_Pin)==GPIO_PIN_SET))
		{
			HAL_GPIO_WritePin(OA_GPIO_Port,OA_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(OB_GPIO_Port,OB_Pin,GPIO_PIN_SET);
		}
		else if((HAL_GPIO_ReadPin(DOWN_GPIO_Port,DOWN_Pin)==GPIO_PIN_RESET)&&
			(HAL_GPIO_ReadPin(UP_GPIO_Port,UP_Pin)==GPIO_PIN_RESET))
		{
			HAL_GPIO_WritePin(OA_GPIO_Port,OA_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(OB_GPIO_Port,OB_Pin,GPIO_PIN_RESET);
			clear_Encoder();
			while(!((HAL_GPIO_ReadPin(DOWN_GPIO_Port,DOWN_Pin)==GPIO_PIN_RESET)&&
			(HAL_GPIO_ReadPin(UP_GPIO_Port,UP_Pin)==GPIO_PIN_RESET)))
			{
				osDelay(100);
			}
			osDelay(200);
			clear_Encoder();
		}
		else
		{
			HAL_GPIO_WritePin(OA_GPIO_Port,OA_Pin,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(OB_GPIO_Port,OB_Pin,GPIO_PIN_RESET);
		}
  }
  /* USER CODE END StartTask_LOGI */
}

/* USER CODE BEGIN Header_StartTask_OLED */
/**
* @brief Function implementing the OLED_DISPLAY thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask_OLED */
void StartTask_OLED(void const * argument)
{
  /* USER CODE BEGIN StartTask_OLED */

	osDelay(10);
  OLED_Init();
	OLED_Fill(0xff);
	OLED_Fill(0x00);
	clear_Encoder();
	char str[15];
	sprintf(str,(char*)"Encoder :%5d",htim2.Instance->CNT);
	OLED_ShowStr(0,0,str,2);
	sprintf(str,(char*)"Rounds  :%5.0lf",get_Encoder());
	OLED_ShowStr(0,2,str,2);
	get_Equation(str);
	OLED_ShowStr(0,4,str,2);
	sprintf(str,(char*)"Distance:%7.1lf",get_Distance());
	OLED_ShowStr(0,6,str,2);
  /* Infinite loop */
  for(;;)
  {
		sprintf(str,(char*)"%5d",htim2.Instance->CNT);
		OLED_ShowStr(72,0,str,2);
		sprintf(str,(char*)"%5.0lf",get_Encoder());
		OLED_ShowStr(72,2,str,2);
		if(get_Equation(str))
			OLED_ShowStr(0,4,str,2);
		sprintf(str,(char*)"%7.1lf",get_Distance());
		OLED_ShowStr(72,6,str,2);
    osDelay(200);
  }
  /* USER CODE END StartTask_OLED */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

