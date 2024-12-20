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
#include "bsp_esp8266.h"
#include "bsp_delay.h"
#include "app_system.h"
#include "key.h"

/*
#define DEBOUNCE_DELAY 1000
#define MAX_MENU_ITEMS 4
int currentMenuIndex = 0;
void DrawMenu(void);
*/

void SystemClock_Config(void);

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
	
	void show_hanzi(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, unsigned char *p)
{
	int i, j;
	for(i = 0; i < 16; i++)//(1) ���ѭ��16�� ---- 16��
	{
		//(2)  �ڲ�ѭ��16�� ----- 16��
	  for(j = 7; j >=0; j--) // ȡ��*p��ֵ�������ư�λ�ж�  8��   *p & (1<<7)
		{
			if(*p & (1<<j))// �ж� *p�� bit7 bit6 .... bit0  
		    {  //���==1 ����( x+, y,ǰ��ɫ)
				Gui_DrawPoint(x+7-j, y, fc);  //��x�� +0 1 2 3 4 ...7 
			}
			else //���==0 ����( x+, y,����ɫ) 
			{ 
			  Gui_DrawPoint(x+7-j, y, bc);  //��x�� +0 1 2 3 4 ...7 
			}
		}
		p++;// p++; ��һ������
	   for(j = 7; j >=0; j--) // ȡ��*p��ֵ�������ư�λ�ж�  8��   *p & (1<<7)
		{
			if(*p & (1<<j))// �ж� *p�� bit7 bit6 .... bit0  
		  {  //���==1 ����( x+, y,ǰ��ɫ)
				Gui_DrawPoint(x+15-j, y, fc);  //��x�� +8 9 10 ...15 
			}
			else //���==0 ����( x+, y,����ɫ) 
			{ 
			  Gui_DrawPoint(x+15-j, y, bc);  //��x�� +8 9 10 ...15  
			}
		}
		p++;  // p++; ��һ������
	  y++;  // y++;  ��һ��
	}
}
	
	
	unsigned char bow[]=
{
/*------------------------------------------------------------------------------
  Դ�ļ� / ���� : 
  ����ߣ����أ�: 16��16
------------------------------------------------------------------------------*/
0x02,0x00,0x05,0x00,0x08,0x80,0x10,0x40,0x20,0x20,0x10,0x10,0x08,0x08,0x04,0x04,
0x02,0x02,0x05,0x01,0x0A,0x82,0x14,0x44,0x28,0x28,0x30,0x10,0x40,0x00,0x00,0x00
};

	unsigned char MENU[]=
{
/*------------------------------------------------------------------------------
  Դ�ļ� / ���� : MENU
  ����ߣ����أ�: 32��16
------------------------------------------------------------------------------*/
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xFC,0xC7,0xE7,
0x6C,0x42,0x62,0x42,0x6C,0x48,0x62,0x42,0x6C,0x48,0x52,0x42,0x6C,0x78,0x52,0x42,
0x6C,0x48,0x4A,0x42,0x54,0x48,0x4A,0x42,0x54,0x40,0x4A,0x42,0x54,0x42,0x46,0x42,
0x54,0x42,0x46,0x42,0xD6,0xFC,0xE2,0x3C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

unsigned char ME[]=
{
/*------------------------------------------------------------------------------
  Դ�ļ� / ���� : ME
  ����ߣ����أ�: 16��16
------------------------------------------------------------------------------*/
0x00,0x00,0x00,0x00,0x00,0x00,0xEE,0xFC,0x6C,0x42,0x6C,0x48,0x6C,0x48,0x6C,0x78,
0x6C,0x48,0x54,0x48,0x54,0x40,0x54,0x42,0x54,0x42,0xD6,0xFC,0x00,0x00,0x00,0x00
};


unsigned char n1[]=
{
/*------------------------------------------------------------------------------
  Դ�ļ� / ���� : 1
  ����ߣ����أ�: 8��16
------------------------------------------------------------------------------*/
0x00,0x00,0x00,0x08,0x38,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x3E,0x00,0x00
};

unsigned char n2[]=
{
/*------------------------------------------------------------------------------
  Դ�ļ� / ���� : 2
  ����ߣ����أ�: 8��16
------------------------------------------------------------------------------*/
0x00,0x00,0x00,0x3C,0x42,0x42,0x42,0x02,0x04,0x08,0x10,0x20,0x42,0x7E,0x00,0x00
};

unsigned char n3[]=
{
/*------------------------------------------------------------------------------
  Դ�ļ� / ���� : 3
  ����ߣ����أ�: 8��16
------------------------------------------------------------------------------*/
0x00,0x00,0x00,0x3C,0x42,0x42,0x02,0x04,0x18,0x04,0x02,0x42,0x42,0x3C,0x00,0x00
};

unsigned char n4[]=
{
/*------------------------------------------------------------------------------
  Դ�ļ� / ���� : 4
  ����ߣ����أ�: 8��16
------------------------------------------------------------------------------*/
0x00,0x00,0x00,0x04,0x0C,0x0C,0x14,0x24,0x24,0x44,0x7F,0x04,0x04,0x1F,0x00,0x00
};


	
/*	
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  Lcd_Init();
  Lcd_Clear(WHITE);

  KeyInit();

  // ��ʼ���Ʋ˵���
  DrawMenu();

  while(1)
  {  
    int key = GetKey();
    if(key == KEY_UP)
    {
      if(currentMenuIndex > 0)  // ����Ƿ��ڶ����߽�
      {
        currentMenuIndex--;  // �����ƶ�
        DrawMenu();  // ���»��Ʋ˵�
      }
    }
    else if(key == KEY_DOWN)
    {
      if(currentMenuIndex < MAX_MENU_ITEMS - 1)  // ����Ƿ��ڵײ��߽�
      {
        currentMenuIndex++;  // �����ƶ�
        DrawMenu();  // ���»��Ʋ˵�
      }
    }
  }
}

void DrawMenu(void)
{
  // ������в˵������ɫ
  Gui_DrawFont_GBK16(0,0,RED,YELLOW,(uint8_t *)"MENU1");
  Gui_DrawFont_GBK16(0,16,RED,YELLOW,(uint8_t *)"MENU2");
  Gui_DrawFont_GBK16(0,32,RED,YELLOW,(uint8_t *)"MENU3");
  Gui_DrawFont_GBK16(0,48,RED,YELLOW,(uint8_t *)"MENU4");

  // ���ݵ�ǰѡ�еĲ˵����������ı�����ɫ
  switch(currentMenuIndex)
  {
    case 0:
      Gui_DrawFont_GBK16(0,0,BLUE,YELLOW,(uint8_t *)"MENU1");
      break;
    case 1:
      Gui_DrawFont_GBK16(0,16,BLUE,YELLOW,(uint8_t *)"MENU2");
      break;
    case 2:
      Gui_DrawFont_GBK16(0,32,BLUE,YELLOW,(uint8_t *)"MENU3");
      break;
    case 3:
      Gui_DrawFont_GBK16(0,48,BLUE,YELLOW,(uint8_t *)"MENU4");
      break;
  }
}
*/

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();

  Lcd_Init();
	Lcd_Clear(WHITE);

	KeyInit();
	
		char *MENU[4]  = {"MENU1","MENU2","MENU3","MENU4"};  
		int key;
		int flag;
		int i;
	
	Gui_DrawFont_GBK16(0,0,RED,YELLOW,(uint8_t *)"MENU1");
	Gui_DrawFont_GBK16(0,16,RED,YELLOW,(uint8_t *)"MENU2");
	Gui_DrawFont_GBK16(0,32,RED,YELLOW,(uint8_t *)"MENU3");
	Gui_DrawFont_GBK16(0,48,RED,YELLOW,(uint8_t *)"MENU4");
	
		while(1)   //��ѭ����
	{
				//��ͣ�ü��  ��û�� �������¡�
			key = GetKey();     //������⵽����֮�� �� ����л��˵����أ�
			if(key == KEY_UP)
			{
						flag --;
						if(flag < 0)
						{
								flag = 3;
						}
			}
			else if(key == KEY_DOWN)
			{
					flag ++;
					if(flag > 3)
					{
							flag = 0;
					}
			}
		  
			for(i = 0; i < 4; i++)   //����forѭ�� �Ѳ˵���ʾ������
			{
					if(i == flag)    //�ж� ��ѡ�е��� ��һ�в˵�
					{
							Gui_DrawFont_GBK16(0,0 + i *16 ,RED,BLUE,(uint8_t *)MENU[i]);
					}
					else 
					{
							Gui_DrawFont_GBK16(0,0 + i * 16 ,BLUE,RED,(uint8_t *)MENU[i]);
					}
			}
			HAL_Delay(100);
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
