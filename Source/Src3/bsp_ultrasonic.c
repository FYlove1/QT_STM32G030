/**
  ******************************************************************************
  * File Name          : ultrasonic.c
  * Description        : 用于超声波测距的初始化和距离的测量。
  ******************************************************************************
  *

  *
  ******************************************************************************
  */
#include "bsp_ultrasonic.h"
#include "tim.h"
#include "gpio.h"
#include "bsp_delay.h"


// 超声波模块 输出口
#define D1_Echo HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7)

// 超声波模块 控制口
#define D2_Trig_H HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET)
#define D2_Trig_L HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET)

void ultrasonicUsing_Init(void)   
{
  MX_GPIO_Init();  

	MX_TIM3_Init();
}    

uint32_t get_ultrasonic_val(void)
{
	uint32_t TIM=0;
	
	D2_Trig_H;       
	delay_us(20);   
	D2_Trig_L;                   
	delay_us(10); 
	
	__HAL_TIM_SET_COUNTER(&htim3,0);

	while(D1_Echo == 0);        //等待Echo回波引脚变高电平
	__HAL_TIM_ENABLE(&htim3);  //开启定时器
	while(D1_Echo == 1);
	__HAL_TIM_DISABLE(&htim3);

	TIM = __HAL_TIM_GET_COUNTER(&htim3);         
    
	return ((TIM * 340)/2) /1000 + 3;   //除以1000，单位mm，3mm的误差
}


