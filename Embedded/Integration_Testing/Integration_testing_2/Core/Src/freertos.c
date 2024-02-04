/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "I2C_LCD.h"
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TRIG_PIN_LEFT GPIO_PIN_8
#define TRIG_PORT_LEFT GPIOA

#define TRIG_PIN_RIGHT GPIO_PIN_5
#define TRIG_PORT_RIGHT GPIOB
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern UART_HandleTypeDef huart1;
//extern UART_HandleTypeDef huart2; // Debug uart to computer select
extern UART_HandleTypeDef huart6;

uint32_t IC_VAL_left_1 = 0;		    // Left Ultra Sonic Value1
uint32_t IC_Val_left_2 = 0;			// Left Ultra Sonic Value2
uint32_t Difference_left = 0;		// Left Ultra Sonic Value2 - Value1
uint8_t Is_First_Captured_left = 0; // Left Ultra Sonic Input Capture Value
uint8_t Distance_left  = 0;			// now Left Ultra Sonic Distance Value

uint32_t IC_Val_right_1 = 0;		 // Right Ultra Sonic Value1
uint32_t IC_Val_right_2 = 0;		 // Right Ultra Sonic Value2
uint32_t Difference_right = 0;		 // Right Ultra Sonic Value2 - Value1
uint8_t Is_First_Captured_right = 0; // Right Ultra Sonic Input Capture Value
uint8_t Distance_right  = 0;		 // now Right Ultra Sonic Distance Value

char buff[20];	// I2C LCD Display string array

uint8_t rxData[100]; // Debuging Uart2(Computer) Receive Data
uint8_t txData[1];	 // Debuging Uart2(Computer) Trasmit Data

uint8_t phone_tx_Data[8];
//    "123A4567"   , PASS/FAIL/WARN, LLM_AI output
// Car number plate, 3 LED Value   , call Manager

uint8_t rasp_tx_dist1_Data[1]; 	 // Sending to rasp in dist1 Value
uint8_t rasp_tx_dist2_Data[1];	 // Sending to rasp in dist2 Value
uint8_t rasp_rx_number_plate[14]; // Receive to rasp in Car number plate / PASS / FAIL
uint8_t pasing_number_plate[9];

enum {DETECT, KEEP};			 // Ultra Sonic DeBounce
uint8_t curState_Left = DETECT;	 // Left Ultra Sonic State
uint8_t curState_Right = DETECT; // Right Ultra Sonic State
uint8_t phone_state = DETECT;
uint8_t lcd_state = 2;

char Pass[] = "PASS";
char Fail[] = "FAIL";
uint8_t cnt_left=0;
uint8_t cnt_left_in =0;
uint8_t cnt_right=0;
uint8_t cnt_right_in = 0;
uint16_t count = 0;

/* USER CODE END Variables */
osThreadId hSemTaskHandle;
osThreadId hSemTask2Handle;
osThreadId hSemTask3Handle;
osThreadId hSemTask4Handle;
osThreadId hSemTask5Handle;
osSemaphoreId hBinarySemHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void delay (uint16_t time)
{
	__HAL_TIM_SET_COUNTER(&htim2, 0);
	while (__HAL_TIM_GET_COUNTER (&htim2) < time);
}


void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)  // if the interrupt source is channel1
//	if (htim == &htim2)
	{
		if (Is_First_Captured_left==0) // if the first value is not captured
		{
			IC_VAL_left_1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1); // read the first value
			Is_First_Captured_left = 1;  // set the first captured as true
			// Now change the polarity to falling edge
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
		}

		else if (Is_First_Captured_left==1)   // if the first is already captured
		{
			IC_Val_left_2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);  // read second value
			__HAL_TIM_SET_COUNTER(htim, 0);  // reset the counter

			if (IC_Val_left_2 > IC_VAL_left_1)
			{
				Difference_left = IC_Val_left_2-IC_VAL_left_1;
			}

			else if (IC_VAL_left_1 > IC_Val_left_2)
			{
				Difference_left = (0xffff - IC_VAL_left_1) + IC_Val_left_2;
			}

			Distance_left = Difference_left * .034/2;
			Is_First_Captured_left = 0; // set it back to false

			// set polarity to rising edge
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);
		}
	}
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3)  // if the interrupt source is channel1
//	if (htim == &htim3)
	{
		if (Is_First_Captured_right==0) // if the first value is not captured
		{
			IC_Val_right_1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3); // read the first value
			Is_First_Captured_right = 1;  // set the first captured as true
			// Now change the polarity to falling edge
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING);
		}

		else if (Is_First_Captured_right==1)   // if the first is already captured
		{
			IC_Val_right_2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);  // read second value
			__HAL_TIM_SET_COUNTER(htim, 0);  // reset the counter

			if (IC_Val_right_2 > IC_Val_right_1)
			{
				Difference_right = IC_Val_right_2-IC_Val_right_1;
			}

			else if (IC_Val_right_1 > IC_Val_right_2)
			{
				Difference_right = (0xffff - IC_Val_right_1) + IC_Val_right_2;
			}

			Distance_right = Difference_right * .034/2;
			Is_First_Captured_right = 0; // set it back to false

			// set polarity to rising edge
			__HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
			__HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC3);
		}
	}
}

void HCSR04_Read_Left (void)
{
	HAL_GPIO_WritePin(TRIG_PORT_LEFT, TRIG_PIN_LEFT, GPIO_PIN_SET);  // pull the TRIG pin HIGH
	delay(10);  // wait for 10 us
	HAL_GPIO_WritePin(TRIG_PORT_LEFT, TRIG_PIN_LEFT, GPIO_PIN_RESET);  // pull the TRIG pin low

	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC1);
}

void HCSR04_Read_Right (void)
{
	HAL_GPIO_WritePin(TRIG_PORT_RIGHT, TRIG_PIN_RIGHT, GPIO_PIN_SET);  // pull the TRIG pin HIGH
	delay(10);  // wait for 10 us
	HAL_GPIO_WritePin(TRIG_PORT_RIGHT, TRIG_PIN_RIGHT, GPIO_PIN_RESET);  // pull the TRIG pin low

	__HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC3);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
//	if(huart->Instance==USART6)	// Checking Uart Debug
//	{
//		HAL_UART_Receive_IT(&huart6, rasp_rx_number_plate, sizeof(rasp_rx_number_plate));
////		HAL_UART_Transmit_IT(&huart2, rxData, sizeof(rxData));
//	}
//	if(huart->Instance==USART1) // Checking Uart Debug
//	{
//		HAL_UART_Receive_IT(&huart1, txData, sizeof(txData));
//		HAL_UART_Transmit_IT(&huart1, txData, sizeof(txData));
//	}
}

/* USER CODE END FunctionPrototypes */

void UltraSonicTask(void const * argument);
void ServoMotorTask(void const * argument);
void LcdTask(void const * argument);
void ManagerTask(void const * argument);
void RaspTask(void const * argument);

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
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  LCD_init();

  TIM3->CCR1 = 110;
  TIM3->CCR2 = 110;

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of hBinarySem */
  osSemaphoreDef(hBinarySem);
  hBinarySemHandle = osSemaphoreCreate(osSemaphore(hBinarySem), 1);

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
  /* definition and creation of hSemTask */
  osThreadDef(hSemTask, UltraSonicTask, osPriorityNormal, 0, 256);
  hSemTaskHandle = osThreadCreate(osThread(hSemTask), NULL);

  /* definition and creation of hSemTask2 */
  osThreadDef(hSemTask2, ServoMotorTask, osPriorityNormal, 0, 256);
  hSemTask2Handle = osThreadCreate(osThread(hSemTask2), NULL);

  /* definition and creation of hSemTask3 */
  osThreadDef(hSemTask3, LcdTask, osPriorityNormal, 0, 256);
  hSemTask3Handle = osThreadCreate(osThread(hSemTask3), NULL);

  /* definition and creation of hSemTask4 */
  osThreadDef(hSemTask4, ManagerTask, osPriorityNormal, 0, 256);
  hSemTask4Handle = osThreadCreate(osThread(hSemTask4), NULL);

  /* definition and creation of hSemTask5 */
  osThreadDef(hSemTask5, RaspTask, osPriorityNormal, 0, 256);
  hSemTask5Handle = osThreadCreate(osThread(hSemTask5), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_UltraSonicTask */
/**
  * @brief  Function implementing the hSemTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_UltraSonicTask */
void UltraSonicTask(void const * argument)
{
  /* USER CODE BEGIN UltraSonicTask */
  /* Infinite loop */
  for(;;)
  {
	  /* ***********Ultra Sonic Check Task*********** */
	  HCSR04_Read_Left();
	  osDelay(100);
	  HCSR04_Read_Right();
	  osDelay(100);
  }
  /* USER CODE END UltraSonicTask */
}

/* USER CODE BEGIN Header_ServoMotorTask */
/**
* @brief Function implementing the hSemTask2 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ServoMotorTask */
void ServoMotorTask(void const * argument)
{
  /* USER CODE BEGIN ServoMotorTask */
  /* Infinite loop */
  for(;;)
  {
	if(strstr(rasp_rx_number_plate, "IASS") != NULL)
    {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RESET);
    	TIM3->CCR1 = 55;
    	osDelay(3000);
		for (int i = 0; i < sizeof(rasp_rx_number_plate); ++i)
		{
			rasp_rx_number_plate[i] = '\0';
		}
		phone_state = DETECT;
    }
    else if(Distance_left > 10)
    {
    	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, RESET);
    	TIM3->CCR1 = 110;
    	osDelay(100);
    }

	if(strstr(rasp_rx_number_plate, "OASS") != NULL)
    {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, SET);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RESET);
    	TIM3->CCR2 = 55;
    	osDelay(3000);
		for (int i = 0; i < sizeof(rasp_rx_number_plate); ++i)
		{
			rasp_rx_number_plate[i] = '\0';
		}
		phone_state = DETECT;
    }
	else if(Distance_right > 10)
    {
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, RESET);
    	TIM3->CCR2 = 110;
    	osDelay(100);
    }
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RESET);
	//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, RESET);
  }
  /* USER CODE END ServoMotorTask */
}

/* USER CODE BEGIN Header_LcdTask */
/**
* @brief Function implementing the hSemTask3 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_LcdTask */
void LcdTask(void const * argument)
{
  /* USER CODE BEGIN LcdTask */
  /* Infinite loop */
  for(;;)
  {
	  /* ***********LCD Display Output Task*********** */



	  /* algorithm change need */

//	count++;
	sprintf(buff, "Parking Manager ");
	LCD_put_cursor(0, 0);
	LCD_send_str(buff);
	sprintf(buff, "                ");
	LCD_put_cursor(1, 0);
	LCD_send_str(buff);
	if (lcd_state == DETECT && (Distance_left < 10 || Distance_right < 10))
	{
		osDelay(3500);
		sprintf(buff, "Welcome         ");
		LCD_put_cursor(0, 0);
		LCD_send_str(buff);
		sprintf(buff, pasing_number_plate);
		LCD_put_cursor(1, 0);
		LCD_send_str(buff);
		lcd_state = KEEP;
		osDelay(3000);
	}

	osDelay(500);
  }
  /* USER CODE END LcdTask */
}

/* USER CODE BEGIN Header_ManagerTask */
/**
* @brief Function implementing the hSemTask4 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_ManagerTask */
void ManagerTask(void const * argument)
{
  /* USER CODE BEGIN ManagerTask */
  /* Infinite loop */
  for(;;)
  {
//	if ((strstr(rasp_rx_number_plate, "IASS") != NULL))
//	{
//		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, SET); // green led
//		osDelay(100);
		// phone_tx_Data input
//		HAL_UART_Transmit_IT(&huart1, Pass, sizeof(Pass));
//		osDelay(10);
//		HAL_UART_Transmit_IT(&huart1, pasing_number_plate, sizeof(pasing_number_plate));
//		osDelay(100);
//		phone_state = KEEP;
//	}
//	if ((strstr(rasp_rx_number_plate, "OASS") != NULL))
//	{
//		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, SET); // green led
//		osDelay(100);
//		// phone_tx_Data input
//		HAL_UART_Transmit_IT(&huart1, Pass, sizeof(Pass));
//		osDelay(10);
//		HAL_UART_Transmit_IT(&huart1, pasing_number_plate, sizeof(pasing_number_plate));
//		osDelay(100);
//		phone_state = KEEP;
//	}
	if (strstr(rasp_rx_number_plate, "IAIL") != NULL )
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, SET); // red led on
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, RESET); // green led off
		HAL_UART_Transmit_IT(&huart1, Fail, sizeof(Fail));
		osDelay(3000);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RESET); // red led off
		// phone_tx_Data input
//		HAL_UART_Transmit_IT(&huart1, pasing_number_plate, sizeof(pasing_number_plate));
//		osDelay(100);
	}
	if (strstr(rasp_rx_number_plate, "OAIL") != NULL)
	{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, SET); // red led
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, RESET); // green led off
		HAL_UART_Transmit_IT(&huart1, Fail, sizeof(Fail));
		osDelay(3000);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, RESET); // red led off
		// phone_tx_Data input
//		HAL_UART_Transmit_IT(&huart1, pasing_number_plate, sizeof(pasing_number_plate));
//		osDelay(100);
	}
  }
  /* USER CODE END ManagerTask */
}

/* USER CODE BEGIN Header_RaspTask */
/**
* @brief Function implementing the hSemTask5 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_RaspTask */
void RaspTask(void const * argument)
{
  /* USER CODE BEGIN RaspTask */
  /* Infinite loop */
  for(;;)
  {
	  /* ***********Sending Data to Rasp Task*********** */
		if (Distance_left < 10 && curState_Left == DETECT)
		{
			rasp_tx_dist1_Data[0] = 'I';
			HAL_UART_Transmit_IT(&huart6, rasp_tx_dist1_Data, sizeof(rasp_tx_dist1_Data));
			osDelay(2000);
			HAL_UART_Receive_IT(&huart6, rasp_rx_number_plate, sizeof(rasp_rx_number_plate));
			cnt_left++;
			osDelay(1500);

			curState_Left = KEEP;

		}
		else if (Distance_left < 10 && curState_Left == KEEP)
		{
			HAL_UART_Receive_IT(&huart6, rasp_rx_number_plate, sizeof(rasp_rx_number_plate));
//			if (rasp_rx_number_plate[0]!='I' || rasp_rx_number_plate[1] != 'A' || rasp_rx_number_plate[2] != 'S' || rasp_rx_number_plate[3] != 'S')
//			{
//				HAL_UART_Transmit_IT(&huart6, rasp_tx_dist1_Data, sizeof(rasp_tx_dist1_Data));
//				osDelay(2000);
//				HAL_UART_Receive_IT(&huart6, rasp_rx_number_plate, sizeof(rasp_rx_number_plate));
//				osDelay(1500);
//				cnt_left_in++;
//			}

			osDelay(100);
		}
		else if(Distance_left > 10 && curState_Left == KEEP)
		{

			curState_Left = DETECT;
			lcd_state = DETECT;
			HAL_UART_Transmit_IT(&huart1, Pass, sizeof(Pass));
			osDelay(10);
			HAL_UART_Transmit_IT(&huart1, pasing_number_plate, sizeof(pasing_number_plate));
			osDelay(100);
		}





		if (Distance_right < 10 && curState_Right == DETECT )
		{
			rasp_tx_dist2_Data[0] = 'O';
			HAL_UART_Transmit_IT(&huart6, rasp_tx_dist2_Data, sizeof(rasp_tx_dist2_Data));
			osDelay(2000);
			HAL_UART_Receive_IT(&huart6, rasp_rx_number_plate, sizeof(rasp_rx_number_plate));
			cnt_right++;
			osDelay(1500);

			curState_Right = KEEP;

//			if(strstr(rasp_rx_number_plate, "OASS") == NULL)
//			{
//				HAL_UART_Transmit_IT(&huart6, rasp_tx_dist2_Data, sizeof(rasp_tx_dist2_Data));
//				osDelay(100);
//				HAL_UART_Receive_IT(&huart6, rasp_rx_number_plate, sizeof(rasp_rx_number_plate));
//				cnt_right_in++;
//			}

		}
		else if (Distance_right < 10 && curState_Right == KEEP)
		{
			HAL_UART_Receive_IT(&huart6, rasp_rx_number_plate, sizeof(rasp_rx_number_plate));
			osDelay(100);
		}
		else if (Distance_right > 10 && curState_Right == KEEP)
		{

			curState_Right = DETECT;
			lcd_state = DETECT;
			HAL_UART_Transmit_IT(&huart1, Pass, sizeof(Pass));
			osDelay(10);
			HAL_UART_Transmit_IT(&huart1, pasing_number_plate, sizeof(pasing_number_plate));
			osDelay(100);
		}

	    //HAL_UART_Receive_IT(&huart6, rasp_rx_number_plate, sizeof(rasp_rx_number_plate));
	    sscanf(rasp_rx_number_plate, "IASS %8[^\n]", pasing_number_plate);
	    sscanf(rasp_rx_number_plate, "OASS %8[^\n]", pasing_number_plate);

	    if(strstr(rasp_rx_number_plate, "IASS") == NULL && strstr(rasp_rx_number_plate, "OASS") == NULL)
	    {
		  memset(rasp_rx_number_plate, 0, sizeof(rasp_rx_number_plate));
	    }

	    rasp_tx_dist1_Data[0] = '\0';
	    rasp_tx_dist2_Data[0] = '\0';
  }
  /* USER CODE END RaspTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */
