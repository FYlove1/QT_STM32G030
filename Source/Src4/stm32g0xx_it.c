/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g0xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32g0xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "stdlib.h"
#include "usart.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M0+ Processor Interruption and Exception Handlers          */ 
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVC_IRQn 0 */

  /* USER CODE END SVC_IRQn 0 */
  /* USER CODE BEGIN SVC_IRQn 1 */

  /* USER CODE END SVC_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G0xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g0xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel 1 interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel 2 and channel 3 interrupts.
  */
void DMA1_Channel2_3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel2_3_IRQn 0 */

  /* USER CODE END DMA1_Channel2_3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
  /* USER CODE BEGIN DMA1_Channel2_3_IRQn 1 */

  /* USER CODE END DMA1_Channel2_3_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt / USART1 wake-up interrupt through EXTI line 25.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */
	if(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE) == SET)   // 触发空闲中断
	{
			uint16_t temp = 0;
			__HAL_UART_CLEAR_IDLEFLAG(&huart1);        // 清除串口1空闲中断标志位
			HAL_UART_DMAStop(&huart1);                 // 关闭DMA
			temp = huart1.Instance->ISR;               // 清除ISR状态寄存器
			temp = huart1.Instance->RDR;               // 读取RDR数据寄存器，用来清除中断
			temp = hdma_usart1_rx.Instance->CNDTR;     // 获取DMA中未传输的数据个数
			Usart1type.UsartDMARecLen = USART_DMA_REC_SIE - temp;         // 总个数 - 未传输的数据个数 = 已经接收数据的个数
			HAL_UART_RxCpltCallback(&huart1);		       // 串口接收回调函数
	}
  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */
	HAL_UART_Receive_DMA(&huart1,Usart1type.UsartDMARecBuffer,USART_DMA_REC_SIE); // 开启串口DMA接收数据
  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt / USART2 wake-up interrupt through EXTI line 26.
  */
void USART2_IRQHandler(void)
{
  /* USER CODE BEGIN USART2_IRQn 0 */
	if(__HAL_UART_GET_FLAG(&huart2,UART_FLAG_IDLE) == SET)   // 触发空闲中断
	{
			uint16_t temp = 0;
			__HAL_UART_CLEAR_IDLEFLAG(&huart2);        // 清除串口2空闲中断标志位
			HAL_UART_DMAStop(&huart2);                 // 关闭DMA
			temp = huart2.Instance->ISR;               // 清除ISR状态寄存器
			temp = huart2.Instance->RDR;               // 读取RDR数据寄存器，用来清除中断
			temp = hdma_usart2_rx.Instance->CNDTR;     // 获取DMA中未传输的数据个数
			Usart2type.UsartDMARecLen = USART_DMA_REC_SIE - temp;         // 总个数 - 未传输的数据个数 = 已经接收数据的个数
			HAL_UART_RxCpltCallback(&huart2);		       // 串口接收回调函数
	}
  /* USER CODE END USART2_IRQn 0 */
  HAL_UART_IRQHandler(&huart2);
  /* USER CODE BEGIN USART2_IRQn 1 */
	HAL_UART_Receive_DMA(&huart2,Usart2type.UsartDMARecBuffer,USART_DMA_REC_SIE); // 开启串口DMA接收数据
  /* USER CODE END USART2_IRQn 1 */
}

/* USER CODE BEGIN 1 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
  {
		// 如果已经有数据长度了
		if(Usart1type.UsartRecLen > 0)           
		{
				// 转存到待处理的区域，如果数据接收比较快的时候，还可以保存一条数据，以防止数据丢失
				memcpy(&Usart1type.UsartRecBuffer[Usart1type.UsartRecLen],
									Usart1type.UsartDMARecBuffer,
									Usart1type.UsartDMARecLen); 
				Usart1type.UsartRecLen +=  Usart1type.UsartDMARecLen;  // 累加数据长度
		}
		else
		{
				// 转存到待处理的区域，如果没有接收数据的时候，可以保存一条数据
				memcpy(Usart1type.UsartRecBuffer,
									Usart1type.UsartDMARecBuffer,
									Usart1type.UsartDMARecLen);    
				Usart1type.UsartRecLen =  Usart1type.UsartDMARecLen; // 获得数据长度
		}
		// 清空DMA缓冲区
		memset(Usart1type.UsartDMARecBuffer, 0x00, sizeof(Usart1type.UsartDMARecBuffer));    
		// 设置接收数据标志位
		Usart1type.UsartRecFlag = 1;
  }
	
	// 串口2
	if(huart->Instance == USART2)
  {
		// 如果已经有数据长度了
		if(Usart2type.UsartRecLen > 0)           
		{
				// 转存到待处理的区域，如果数据接收比较快的时候，还可以保存一条数据，以防止数据丢失
				memcpy(&Usart2type.UsartRecBuffer[Usart2type.UsartRecLen],
									Usart2type.UsartDMARecBuffer,
									Usart2type.UsartDMARecLen); 
				Usart2type.UsartRecLen +=  Usart2type.UsartDMARecLen;  // 累加数据长度
		}
		else
		{
				// 转存到待处理的区域，如果没有接收数据的时候，可以保存一条数据
				memcpy(Usart2type.UsartRecBuffer,
									Usart2type.UsartDMARecBuffer,
									Usart2type.UsartDMARecLen);    
				Usart2type.UsartRecLen =  Usart2type.UsartDMARecLen; // 获得数据长度
		}
		// 清空DMA缓冲区
		memset(Usart2type.UsartDMARecBuffer, 0x00, sizeof(Usart2type.UsartDMARecBuffer));    
		// 设置接收数据标志位
		Usart2type.UsartRecFlag = 1;
  }
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
