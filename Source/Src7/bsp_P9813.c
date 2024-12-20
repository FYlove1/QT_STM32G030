#include "bsp_P9813.h"
#include "bsp_delay.h"




#define RGB_DATA_H HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET)
#define RGB_DATA_L HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET)

#define RGB_CLK_H HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET)
#define RGB_CLK_L HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET)

void rgb_Gpio_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PBPin PBPin */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
uint32_t Color_Data(uint8_t Red, uint8_t Green, uint8_t Blue) //写入三个八位的值，返回一个32位的数值
{
  uint32_t temp = 0;

  temp |= (0x03 << 30);           // 0xc0--> 1100 0000     1010 1010
  temp |= ((~Green) & 0xc0) << 22; //取蓝色最高两位取反,得到B7’ B6’
  temp |= ((~Blue) & 0xc0) << 20; //取绿色最高两位取反,得到G7’G6’
  temp |= ((~Red) & 0xc0) << 18; //取绿色最高两位取反,得到R7’R6’
  temp |= Green << 16;
  temp |= Blue << 8;
  temp |= Red << 0;

  return temp;
}

void RGB_Send_Data(uint32_t data)
{
  unsigned int i;
  for (i = 0; i < 32; i++)
  {
    //取data的最高位，如果是1就写1（注意按位操作是写16进制）
    if (data & 0x80000000) //准备一位数据
      RGB_DATA_H;
    else
      RGB_DATA_L;            //否则写0

    data <<= 1; //每取出一个数据，让次高位变成最高位，遍历整个32位的data值
    RGB_CLK_L;
    delay_us(10);
    RGB_CLK_H;
    delay_us(10);
    /*以上四步操作令CLK产生上升沿*/
  }
}

void rgb_setValue(uint8_t Red, uint8_t Green, uint8_t Blue)
{
  uint32_t color_data;
  color_data = Color_Data(Red, Green, Blue);

  RGB_Send_Data(0);          //先发送32位低电平起始信号
  RGB_Send_Data(color_data); //发送第一个32位灰度数据
  RGB_Send_Data(color_data); //发送第二个32位灰度数据
}
