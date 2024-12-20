#include "app_system.h"
#include "adc.h"
#include "gpio.h"
#include "bsp_beep.h"
#include "bsp_lcd.h"
#include "bsp_dht11.h"
#include "bsp_ultrasonic.h"
#include "tim.h"


// 全局参数初始化
Global_Data_t global_Data = {.SwitchValue = SWITCH_OFF};

// DMA数据缓冲区
uint32_t ADC_DMA_Value_Buff[ADC_NUM] = {0};

// M0读取传感板D7引脚（核心板P0_1）的AD值，通过AD值进行判断核心板类型
static uint32_t Read_ADC_CoreType(void)
{
  uint32_t adcCoreTypes = 0;
  //开始DMA转换ADC
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)ADC_DMA_Value_Buff, ADC_NUM);

  HAL_Delay(200);

  adcCoreTypes = (uint16_t)ADC_DMA_Value_Buff[ADC_IN5]; //获得通道5的ADC值

  adcCoreTypes = (adcCoreTypes * 3300) / 4096; //换算ADC的值%d mV,

  adcCoreTypes = adcCoreTypes - 70; //读AD值与比较值有偏差

  printf("CoreADC:%d mV\n", adcCoreTypes);
  return adcCoreTypes;
}

// 板级初始化
void BSP_Init(void)
{
  uint32_t ADC_CoreType_Val = Read_ADC_CoreType();
  if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_FAN - ADC_ERRORS, DEVICE_ADC_FAN + ADC_ERRORS)) //风扇 理论AD采集值 16
  {
    global_Data.DeviceType = DEVICE_TYPE_FAN;
    global_Data.DeviceAddr = DEVICE_ADDR_FAN;
    D1_OUT_GPIO_Init();
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_LIGHT - ADC_ERRORS, DEVICE_ADC_LIGHT + ADC_ERRORS)) //光强 理论AD采集值 65
  {
    global_Data.DeviceType = DEVICE_TYPE_LIGHT;
    global_Data.DeviceAddr = DEVICE_ADDR_LIGHT;
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_TEMPHUMI - ADC_ERRORS, DEVICE_ADC_TEMPHUMI + ADC_ERRORS)) //温湿度 理论AD采集值105
  {
    global_Data.DeviceType = DEVICE_TYPE_TEMPHUMI;
    global_Data.DeviceAddr = DEVICE_ADDR_TEMPHUMI;
    D2_OUT_GPIO_Init();
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_GAS - ADC_ERRORS, DEVICE_ADC_GAS + ADC_ERRORS)) //可燃气体 理论AD采集值300
  {
    global_Data.DeviceType = DEVICE_TYPE_GAS;
    global_Data.DeviceAddr = DEVICE_ADDR_GAS;
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_FLAME - ADC_ERRORS, DEVICE_ADC_FLAME + ADC_ERRORS)) //火焰  理论AD采集值 430
  {
    global_Data.DeviceType = DEVICE_TYPE_FLAME;
    global_Data.DeviceAddr = DEVICE_ADDR_FLAME;
    D2_IN_GPIO_Init();
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_PHOTOACTOR - ADC_ERRORS, DEVICE_ADC_PHOTOACTOR + ADC_ERRORS)) //光电开关 理论AD采集值 550
  {
    global_Data.DeviceType = DEVICE_TYPE_PHOTOACTOR;
    global_Data.DeviceAddr = DEVICE_ADDR_PHOTOACTOR;
    D2_IN_GPIO_Init();
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_LAMP - ADC_ERRORS, DEVICE_ADC_LAMP + ADC_ERRORS)) // RGB三色灯 理论AD采集值627
  {
    global_Data.DeviceType = DEVICE_TYPE_LAMP;
    global_Data.DeviceAddr = DEVICE_ADDR_LAMP;
    D1_OUT_GPIO_Init();
    D2_OUT_GPIO_Init();
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_RELAY - ADC_ERRORS, DEVICE_ADC_RELAY + ADC_ERRORS)) //继电器 理论AD采集值700
  {
    global_Data.DeviceType = DEVICE_TYPE_RELAY;
    global_Data.DeviceAddr = DEVICE_ADDR_RELAY;
    D1_OUT_GPIO_Init();
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_BEEP - ADC_ERRORS, DEVICE_ADC_BEEP + ADC_ERRORS)) //蜂鸣器 理论AD采集值818
  {
    global_Data.DeviceType = DEVICE_TYPE_BEEP;
    global_Data.DeviceAddr = DEVICE_ADDR_BEEP;
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_INFRARED - ADC_ERRORS, DEVICE_ADC_INFRARED + ADC_ERRORS)) //人体红外 理论AD采集值1185
  {
    global_Data.DeviceType = DEVICE_TYPE_INFRARED;
    global_Data.DeviceAddr = DEVICE_ADDR_INFRARED;
    D2_IN_GPIO_Init();
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_ULTRASONIC - ADC_ERRORS, DEVICE_ADC_ULTRASONIC + ADC_ERRORS)) //超声波  理论AD采集值1336
  {
    global_Data.DeviceType = DEVICE_TYPE_ULTRASONIC;
    global_Data.DeviceAddr = DEVICE_ADDR_ULTRASONIC;
    ultrasonicUsing_Init();
  }
  else if (ADCCOMP(ADC_CoreType_Val, DEVICE_ADC_TOUCH - ADC_ERRORS, DEVICE_ADC_TOUCH + ADC_ERRORS)) //触摸按键 理论AD采集值1487
  {
    global_Data.DeviceType = DEVICE_TYPE_TOUCH;
    global_Data.DeviceAddr = DEVICE_ADDR_TOUCH;
    D1_IN_GPIO_Init();
  }
  else
  {
    global_Data.DeviceType = DEVICE_TYPE_ERR;
  }
}

// LCD界面初始化
void DisplayDeviceLogo(void)
{
  Lcd_Init();
  Lcd_Clear(YELLOW);
  Gui_DrawFont_GBK16(0, 0, RED, CYAN, (uint8_t *)"  WiFi Topology ");
  Gui_DrawFont_GBK16(0, 16, RED, CYAN, (uint8_t *)"     Station    ");
  Gui_DrawFont_GBK16(0, 32, BLACK, YELLOW, (uint8_t *)"SSID:");
  Gui_DrawFont_GBK16(0, 48, BLACK, YELLOW, (uint8_t *)"IP:");

  switch (global_Data.DeviceType)
  {
  case DEVICE_TYPE_FAN:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Fan:");
    break;

  case DEVICE_TYPE_LIGHT:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Light:");
    break;

  case DEVICE_TYPE_TEMPHUMI:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Temp:");
    Gui_DrawFont_GBK16(0, 80, BLACK, YELLOW, (uint8_t *)"Humidity:");
    break;

  case DEVICE_TYPE_GAS:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Gas:");
    break;

  case DEVICE_TYPE_PHOTOACTOR:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Photoactor:");
    break;

  case DEVICE_TYPE_LAMP:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"RED:");
    Gui_DrawFont_GBK16(0, 80, BLACK, YELLOW, (uint8_t *)"GREEN:");
    Gui_DrawFont_GBK16(0, 96, BLACK, YELLOW, (uint8_t *)"BLUE:");
    break;

  case DEVICE_TYPE_RELAY:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Relay:");
    break;

  case DEVICE_TYPE_BEEP:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Beep:");
    break;

  case DEVICE_TYPE_INFRARED:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Infrared:");
    break;

  case DEVICE_TYPE_ULTRASONIC:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Ultrasonic:");
    break;

  case DEVICE_TYPE_TOUCH:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Touch:");
    break;

  case DEVICE_TYPE_FLAME:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"Flame:");
    break;

  default:
    Gui_DrawFont_GBK16(0, 64, BLACK, YELLOW, (uint8_t *)"  No Sensor  ");
    break;
  }
}

// LCD显示传感器数据
void DisplayDeviceData(void)
{
  char DisplayBuff[30] = {0};

  switch (global_Data.DeviceType)
  {
  case DEVICE_TYPE_FAN: // 风扇
    (global_Data.SwitchValue == SWITCH_ON) ? Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"ON ") : Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"OFF");
    break;

  case DEVICE_TYPE_LIGHT: // 光照
    memset(DisplayBuff, 0, sizeof(DisplayBuff));
    sprintf(DisplayBuff, "%04d", global_Data.ADCValue);
    Gui_DrawFont_GBK16(94, 64, BLACK, YELLOW, (uint8_t *)DisplayBuff);
    break;

  case DEVICE_TYPE_TEMPHUMI: // 温湿度
    memset(DisplayBuff, 0, sizeof(DisplayBuff));
    sprintf(DisplayBuff, "%02d", global_Data.TempValue);
    Gui_DrawFont_GBK16(94, 64, BLACK, YELLOW, (uint8_t *)DisplayBuff);

    memset(DisplayBuff, 0, sizeof(DisplayBuff));
    sprintf(DisplayBuff, "%02d", global_Data.HumiValue);
    Gui_DrawFont_GBK16(94, 80, BLACK, YELLOW, (uint8_t *)DisplayBuff);
    break;

  case DEVICE_TYPE_GAS: // 可燃气
    memset(DisplayBuff, 0, sizeof(DisplayBuff));
    sprintf(DisplayBuff, "%04d", global_Data.ADCValue);
    Gui_DrawFont_GBK16(94, 64, BLACK, YELLOW, (uint8_t *)DisplayBuff);
    break;

  case DEVICE_TYPE_PHOTOACTOR: // 光电开关
    (global_Data.SwitchValue == SWITCH_ON) ? Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"ON ") : Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"OFF");
    break;

  case DEVICE_TYPE_LAMP: // RGB灯
    memset(DisplayBuff, 0, sizeof(DisplayBuff));
    sprintf(DisplayBuff, "%03d", global_Data.RedValue);
    Gui_DrawFont_GBK16(94, 64, BLACK, YELLOW, (uint8_t *)DisplayBuff);

    memset(DisplayBuff, 0, sizeof(DisplayBuff));
    sprintf(DisplayBuff, "%03d", global_Data.GreenValue);
    Gui_DrawFont_GBK16(94, 80, BLACK, YELLOW, (uint8_t *)DisplayBuff);

    memset(DisplayBuff, 0, sizeof(DisplayBuff));
    sprintf(DisplayBuff, "%03d", global_Data.BlueValue);
    Gui_DrawFont_GBK16(94, 96, BLACK, YELLOW, (uint8_t *)DisplayBuff);
    break;

  case DEVICE_TYPE_RELAY: // 继电器
    (global_Data.SwitchValue == SWITCH_ON) ? Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"ON ") : Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"OFF");
    break;

  case DEVICE_TYPE_BEEP: // 蜂鸣器
    (global_Data.SwitchValue == SWITCH_ON) ? Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"ON ") : Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"OFF");
    break;

  case DEVICE_TYPE_INFRARED: // 人体红外传感器
    (global_Data.SwitchValue == SWITCH_ON) ? Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"ON ") : Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"OFF");
    break;

  case DEVICE_TYPE_ULTRASONIC: // 超声波
    memset(DisplayBuff, 0, sizeof(DisplayBuff));
    sprintf(DisplayBuff, "%04d", global_Data.Distance);
    Gui_DrawFont_GBK16(94, 64, BLACK, YELLOW, (uint8_t *)DisplayBuff);
    break;

  case DEVICE_TYPE_TOUCH: // 触摸
    (global_Data.SwitchValue == SWITCH_ON) ? Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"ON ") : Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"OFF");
    break;

  case DEVICE_TYPE_FLAME: // 火焰
    (global_Data.SwitchValue == SWITCH_ON) ? Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"ON ") : Gui_DrawFont_GBK16(102, 64, BLACK, YELLOW, (uint8_t *)"OFF");
    break;

  default:
    break;
  }
}

// 获取传感器的数据
void get_SensorData(void)
{
	uint8_t humiH, humiL, tempH, tempL;
  uint32_t Adc_tmp;
  Adc_tmp = ADC_DMA_Value_Buff[ADC_IN0]; // 获取电池的电压值
  Adc_tmp = (Adc_tmp * 3300) / 4096;     //换算ADC的值%d mV,
  /*我们规定最低放电电压为3.51v ,经过分压AD检测到的应该是2.34v*/
  /*我们规定最低放电电压为4.20v ,经过分压AD检测到的应该是2.8v*/
  //计算百分比   满电的时候是2.8v  ,该充电的时候是2.34v, 460是满电2800与没电2340的差
  global_Data.PowerValue = ((Adc_tmp - 2340) * 100) / 460;

  switch (global_Data.DeviceType)
  {
    // 火焰模块
  case DEVICE_TYPE_FLAME:
    global_Data.SwitchValue = (D2_READ_VAL == GPIO_PIN_SET) ? SWITCH_ON : SWITCH_OFF;
    break;

    // 人体红外
  case DEVICE_TYPE_INFRARED:
    global_Data.SwitchValue = (D2_READ_VAL == GPIO_PIN_SET) ? SWITCH_ON : SWITCH_OFF;
    break;

    // 触摸
  case DEVICE_TYPE_TOUCH:
    global_Data.SwitchValue = (D1_READ_VAL == GPIO_PIN_SET) ? SWITCH_ON : SWITCH_OFF;
    break;

    // 光电开关
  case DEVICE_TYPE_PHOTOACTOR:
    global_Data.SwitchValue = (D2_READ_VAL == GPIO_PIN_SET) ? SWITCH_ON : SWITCH_OFF;
    break;

    // 可燃气体
  case DEVICE_TYPE_GAS:
    Adc_tmp = ADC_DMA_Value_Buff[ADC_IN4];
    global_Data.ADCValue = (Adc_tmp * 3300) / 4096; //换算ADC的值%d mV,
    break;

    // 光照
  case DEVICE_TYPE_LIGHT:
    Adc_tmp = ADC_DMA_Value_Buff[ADC_IN4];
    global_Data.ADCValue = (Adc_tmp * 3300) / 4096; //换算ADC的值%d mV,
    break;

    // 温湿度
  case DEVICE_TYPE_TEMPHUMI:
    if(HAL_OK == DHT11_Read_Data(&humiH, &humiL, &tempH, &tempL))
    {
      global_Data.TempValue = tempH;
      global_Data.HumiValue = humiH;
    }
    break;

    // 超声波
  case DEVICE_TYPE_ULTRASONIC:
    global_Data.Distance = get_ultrasonic_val() / 10; // 转换为cm
    break;

  default:
    break;
  }
}

// 1S产生一个中断信号
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if(htim->Instance == TIM1)
  {
    global_Data.SendDataTime_Flag = 1;
    
  }
}





