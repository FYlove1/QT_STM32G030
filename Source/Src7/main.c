/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_lcd.h"
#include "bsp_ultrasonic.h"
#include "esp8266.h"
#include "bsp_delay.h"
#include "app_system.h"
#include "key.h"
#include "dht11.h"
void SystemClock_Config(void);



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
	delay_init(64);
  MX_GPIO_Init();
	MX_DMA_Init();		//ADC数据读取用到了DMA
  MX_ADC1_Init();		//初始化ADC
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)ADC_DMA_Value,ADC_NUM);//开始转换ADC 	
  Lcd_Init();
  Lcd_Clear(YELLOW) ;
	KeyInit();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
	
  Gui_DrawFont_GBK16(0,32,BLACK,YELLOW,(uint8_t*)"SSID:");
		
  __HAL_UART_ENABLE_IT(&huart2, UART_IT_IDLE);	// 开启串口2的空闲中断
  __HAL_UART_CLEAR_IDLEFLAG(&huart2);  // 清空串口2的空闲中断状态
	// 开启串口2的DMA接收传输
  HAL_UART_Receive_DMA(&huart2,Usart2type.UsartDMARecBuffer,USART_DMA_REC_SIE);
	
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);		// 开启使用USART+DMA的方式接收串口的不定长数据
  __HAL_UART_CLEAR_IDLEFLAG(&huart1);
  HAL_UART_Receive_DMA(&huart1,Usart1type.UsartDMARecBuffer,USART_DMA_REC_SIE);
	
  initEsp8266(); //ESP8266_Init();	// 状态机初始化
	FS_DHT11_Init(); //

	Gui_DrawFont_GBK16(40,32,BLACK,YELLOW,(uint8_t*)WIFI_SSID);  //屏幕显示wifi名
	Gui_DrawFont_GBK16(0,48,BLACK,YELLOW,(uint8_t*)StationIPV4);   //屏幕显示IP地址
  while (1)
  {
		/*HAL_UART_Transmit(&huart2,(uint8_t *)"hello",5,10);  //串口2发送hello
		HAL_Delay(1000);*/
	/*	if(Usart2type.UsartRecFlag)
		{
			Gui_DrawFont_GBK16(0,70, BLACK, YELLOW, Usart2type.UsartRecBuffer); //将接收数据显示到屏幕
			Usart2type.UsartRecLen = 0;
			memset(Usart2type.UsartRecBuffer, 0, sizeof(Usart2type.UsartRecBuffer));
			
		}	
		*/
		
					//接收到的数据 就会显示到 液晶屏幕上。
			//网络调试助手 发送LED2ON (指令)  远程(无线同学)控制  小灯开关
			//LED2OFF  小灯关闭   LED3ON  LED3OFF LED4ON  LED4OFF 
			//如果LED2ON  和 我们接收数据的缓冲区内容一致。
	/*		if(!strcpy(Usart2type.UsartRecBuffer,"ON"))    //字符串比较
			{
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET);
			}
			else if(!strcpy(Usart2type.UsartRecBuffer,"OFF"))
			{
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);
			} */
			
				
		char buf[100] = { 0 };
		char wet[100] = { 0 };
		DHT11_Read_Data();
		if(ucharT_data_H != 0)
				sprintf(buf, "temp:%d.%d", ucharT_data_H, ucharT_data_L);
				Gui_DrawFont_GBK16(10,0,BLACK,YELLOW,buf);
				sprintf(wet, "hum:%d.%d", ucharRH_data_H, ucharRH_data_L);
        Gui_DrawFont_GBK16(10,16,BLACK,YELLOW,wet);
        HAL_Delay(100);
		
		
		
		//HAL_UART_Transmit(&huart2,(uint8_t *)"hello",5,10);  //′??ú2·￠?íhello
		HAL_Delay(1000);
		HAL_UART_Transmit(&huart2,(uint8_t *)buf,20,10);
		HAL_UART_Transmit(&huart2,(uint8_t *)wet,20,10);
			
				Usart2type.UsartRecLen = 0;
			memset(Usart2type.UsartRecBuffer, 0, sizeof(Usart2type.UsartRecBuffer));
			
	  Gui_DrawFont_GBK16(0,70, BLACK, YELLOW, Usart2type.UsartRecBuffer); //将接收数据显示到屏幕
	
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage 
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the peripherals clocks 
  */
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

int fputc(int ch, FILE *f)
{
	#if DEBUG_Printf
		HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 100);
	#endif
  return ch;
}


void Blink_Leds(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint8_t ms)
{
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
  HAL_Delay(ms);
  HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
  HAL_Delay(ms);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
