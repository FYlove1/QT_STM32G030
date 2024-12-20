#include "key.h"
#include "stm32g0xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "app_system.h"

void KeyInit()
{
  MX_DMA_Init();
  MX_ADC1_Init();

  HAL_ADC_Start_DMA(&hadc1,(uint32_t*)ADC_DMA_Value,ADC_NUM);//¿ªÊ¼×ª»»ADC       
}
unsigned int GetKey()
{
    unsigned int  ret = AdSensor_ch1();
    if( ret > 200 && ret < 600 )
    {
      return KEY_DOWN;
    }
    else if(ret > 1064 && ret < 1500)
    {
      return KEY_LEFT;
    }
    else if(ret > 1550 && ret <1800)
    {
      return KEY_UP;
    }
    else if(ret > 1810 && ret < 2285)
    {
      return KEY_SEL;
    }
    else
    {
      return KEY_NONE;
    }
}
