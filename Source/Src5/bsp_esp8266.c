#include "bsp_esp8266.h"
#include "usart.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "bsp_lcd.h"
#include "bsp_softwareTimer.h"
#include "fs_protocol.h"
#include "app_system.h"


static tsTimeType timeESP, timeSendData; // 软件定时器
teESP_TaskStatus ESP_TaskStatus;         // 模组任务状态
static teATCmdNum ATNextCmdNum;          // 下条指令
static teATCmdNum ATCurrentCmdNum;       // 当前指令
static teATCmdNum ATRecCmdNum;           // 接收指令
static uint8_t CurrentRty;               // 重发次数

#define WIFI_SSID "1023"            // WiFi热点的SSID号
#define WIFI_PASSWD "1234567890123" // WiFi热点的密码

#define UDP_SER_IP "255.255.255.255" // IP地址

static char *LOCAL_WIFI_IP = NULL; // 本地IP地址

tsATCmds ATCmds[] = {
    {"AT\r\n", "OK", 500, NO_REC, 20},                                      // 查看ESP8266是否正常运行
    {"AT+CWMODE=1\r\n", "OK", 500, NO_REC, 20},                             // 设置为station模式
    {"AT+CWJAP=\"farsight_%s\",\"%s\"\r\n", "OK", 3000, NO_REC, 20},        // 连接AP热点
    {"AT+CIFSR\r\n", "OK", 500, NO_REC, 5},                                 // 查询本地IP地址
		
    {"AT+CIPSTART=\"UDP\",\"%s\",20000,20000,0\r\n", "OK", 1000, NO_REC, 20}, 	//设置UDP服务端模式
		{"AT+CIPMUX=0\r\n", "OK", 500, NO_REC, 20},	// 配置WiFi为单链接模式
    {"AT+CIPMODE=1\r\n", "OK", 500, NO_REC, 20},                            // 开启透传模式
    {"AT+CIPSEND\r\n", "AT+CIPSEND\r\r\n\r\nOK\r\n\r\n>", 500, NO_REC, 20}, // 开始透传
		
		{"+++", "+++", 2000, NO_REC, 3},						// 关闭透传数据
    {"AT+CIPMODE=0\r\n", "OK", 500, NO_REC, 3},	// 关闭透传模式
    {"AT+CIPCLOSE\r\n", "OK", 500, NO_REC, 3}, 	// 断开连接
};

/**
 * 函数功能： 初始化 ESP8266-WiFi模组 相关变量
 * 参数：
 *      无
 * 返回值：
 *      无
 */
void ESP8266_Init(void)
{
  printf("ESP8266_Init...\r\n");
	
	Gui_DrawFont_GBK16(94, 32, BLACK, YELLOW, (uint8_t *)WIFI_SSID);
	
  ESP_TaskStatus = ESP_SEND;                        //进入发送模式
  ATCurrentCmdNum = AT;                             //当前指令
  ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1); //下一条指令
}
/**
 * 函数功能： ESP8266 发送AT指令数据
 * 参数：
 *      @ATCmdNum： AT指令枚举，与前面AT指令对应
 * 返回值：
 *      无
 */
static void ATSend(teATCmdNum ATCmdNum)
{
  memset(Usart2type.UsartRecBuffer, 0, USART_REC_SIE);
  ATCmds[ATCmdNum].ATStatus = NO_REC; //设置为 未接收
  ATRecCmdNum = ATCmdNum;
  char AT_SendBuff[100] = {0};

  // 连接AP热点
  if (ATCmdNum == AT_CWJAP)
  {
    sprintf(AT_SendBuff, ATCmds[ATCmdNum].ATSendStr, WIFI_SSID, WIFI_PASSWD);
    HAL_UART_Transmit(&huart2, (uint8_t *)AT_SendBuff, strlen(AT_SendBuff), 100);
    printf("SendDataBuff:%s\r\n", AT_SendBuff);
  }
  // 建立服务器
  else if (ATCmdNum == AT_CIPSTART)
  {
    sprintf(AT_SendBuff, ATCmds[ATCmdNum].ATSendStr, UDP_SER_IP);
    HAL_UART_Transmit(&huart2, (uint8_t *)AT_SendBuff, strlen(AT_SendBuff), 100);
    printf("SendDataBuff:%s\r\n", AT_SendBuff);
  }
  else
  {
    HAL_UART_Transmit(&huart2, (uint8_t *)ATCmds[ATCmdNum].ATSendStr, strlen(ATCmds[ATCmdNum].ATSendStr), 0xff);
    printf("SendDataBuff:%s\r\n", ATCmds[ATCmdNum].ATSendStr);
  }

  //打开超时定时器
  setTime(&timeESP, ATCmds[ATCmdNum].TimeOut);
}
/**
 * 函数功能： ESP8266 接收数据
 * 参数：
 *      无
 * 返回值：
 *      无
 */
static void ATRecv(void)
{
  if (Usart2type.UsartRecFlag) // 串口接收到数据
  {
    printf("RecvStr: %s, Rev: %s\r\n", ATCmds[ATRecCmdNum].ATRecStr, Usart2type.UsartRecBuffer);

    if (strstr((const char *)Usart2type.UsartRecBuffer, ATCmds[ATRecCmdNum].ATRecStr) != NULL) //如果包含对应校验符
    {
      ATCmds[ATRecCmdNum].ATStatus = SUCCESS_REC; //设置为成功接收
    }

    // 如果是查询IP地址成功了
    if ((ATRecCmdNum == AT_CIFSR) && (ATCmds[AT_CIFSR].ATStatus == SUCCESS_REC))
    {
      char *pStrstr = NULL;
      char *pStrtok = NULL;
      if ((pStrstr = strstr((const char *)Usart2type.UsartRecBuffer, "+CIFSR:STAIP")) != NULL)
      {
        pStrtok = strtok(pStrstr, "\"");
        printf("========= pStrtok: %s\r\n", pStrtok);

        LOCAL_WIFI_IP = strtok(NULL, "\"");
        printf("========= Local WiFi IP: %s\r\n", LOCAL_WIFI_IP);
        Gui_DrawFont_GBK16(30, 48, BLACK, YELLOW, (uint8_t *)LOCAL_WIFI_IP);

        LED_NET_ON;

        pStrstr = NULL;
        pStrtok = NULL;
      }
    }
    Usart2type.UsartRecFlag = 0; //清空接收标志位
    Usart2type.UsartRecLen = 0;  //清空接收长度
    memset(Usart2type.UsartRecBuffer, 0, sizeof(Usart2type.UsartRecBuffer));
  }
}

/**
 * 函数功能： ESP8266 处理成功命令与失败命令
 * 参数：
 *      无
 * 返回值：
 *      无
 */
static void Rec_WaitAT(void)
{

  if (ATCmds[ATCurrentCmdNum].ATStatus == SUCCESS_REC) // 成功接收到数据包
  {
    switch (ATCurrentCmdNum)
    {
    case AT: // AT指令测试成功
      printf("\r\n AT success...\r\n");
      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CWMODE: // 设置WiFi工作模式
      printf("\r\n AT_CWMODE success...\r\n");
      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CWJAP: // 连接AP热点
      printf("\r\n AT_CWJAP success...\r\n");
      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CIFSR: // 查询本地IP地址
      printf("\r\n AT_CIFSR success...\r\n");

      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CIPSTART: // 建立连接
      printf("\r\n AT_CIPSTART success...\r\n");

      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CIPMODE_1: // 设置透传模式
      printf("\r\n AT_CIPMODE success...\r\n");
      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      break;

    case AT_CIPSEND: // 进入透传模式成功

      printf("\r\n AT_CIPSEND success...\r\n");
      ESP_TaskStatus = ESP_IDIE;
			
			LED_NET_ON;
			
      setTime(&timeSendData, 500);
      break;

    default:
      ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND; // 发送下一条命令
      break;
    }
  }

  else if (compareTime(&timeESP)) //还没收到预期数据  并且超时
  {
    printf("CurrentRty: %d, CmdNum: %d\r\n", CurrentRty, ATCurrentCmdNum);
		
		switch(ATCurrentCmdNum)
		{
			case AT_CWJAP:
				LED_NET_ON;
				HAL_Delay(100);
				LED_NET_OFF;
				LED_RECV_ON;
				HAL_Delay(50);
				LED_RECV_OFF;
				LED_SEND_ON;
				HAL_Delay(50);
				LED_SEND_OFF;
			break;
			default: 
			break;
		}
		
    ATCmds[ATCurrentCmdNum].ATStatus = TIME_OUT;
		LED_NET_OFF;
    if (CurrentRty > 0) //重发
    {
      CurrentRty--;
      printf("now:%d,trytimes:%d\r\n", ATCurrentCmdNum, CurrentRty);
      ATNextCmdNum = ATCurrentCmdNum; //下一条 还是当前命令  实现重发效果
      ESP_TaskStatus = ESP_SEND;
      return;
    }
    else //重发次数用完
    {
      ATCurrentCmdNum = AT;
      ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
      ESP_TaskStatus = ESP_SEND;
      return;
    }
  }
}

/**
 * 函数功能： ESP8266 任务的状态机
 * 参数：
 *      无
 * 返回值：
 *      无
 */
void ESP_TASK(void)
{
  while (1)
  {
    switch (ESP_TaskStatus)
    {
    case ESP_IDIE: // 空闲状态

      SEND_TASK(); // 空闲状态
      return;

    case ESP_SEND:                         // 发送状态 发送ESP8266的AT指令
      if (ATCurrentCmdNum != ATNextCmdNum) // 不是重发状态
      {
        CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum; //重发次数赋值
      }

      ATSend(ATCurrentCmdNum); //发送数据
      ESP_TaskStatus = ESP_WAIT;
      return;
    case ESP_WAIT: //等待数据状态 接收ESP8266的AT指令返回的数据

      ATRecv();     // 接收数据
      Rec_WaitAT(); //处理成功命令与失败命令
      return;
    default:
      // 其他
      return;
    }
  }
}

static uint8_t SendBuff[50] = {0};
static int SendLenth = 0;
/**
 * 函数功能： ESP8266 发送数据任务
 * 参数：
 *      无
 * 返回值：
 *      无
 */
void SEND_TASK(void)
{
  if (ESP_TaskStatus == ESP_IDIE)
  {
    // 每隔一秒，上报一次数据
    if (global_Data.SendDataTime_Flag)
    {
      global_Data.SendDataTime_Flag = 0;

      Blink_Leds(LED_SEND_GPIO_Port, LED_SEND_Pin, 50);

      // 获取传感器数据
      get_SensorData();

      // LCD显示屏显示
      DisplayDeviceData();

      // 打包数据
      memset(SendBuff, 0, sizeof(SendBuff));
     if ((global_Data.DeviceType == DEVICE_TYPE_BEEP) || (global_Data.DeviceType == DEVICE_TYPE_FAN) || (global_Data.DeviceType == DEVICE_TYPE_RELAY) ||
         (global_Data.DeviceType == DEVICE_TYPE_INFRARED) || (global_Data.DeviceType == DEVICE_TYPE_PHOTOACTOR) || (global_Data.DeviceType == DEVICE_TYPE_FLAME) ||
         (global_Data.DeviceType == DEVICE_TYPE_TOUCH))
     {
       SendLenth = packet_data(SendBuff, (uint16_t)global_Data.DeviceAddr, (uint8_t)global_Data.DeviceType, &global_Data.SwitchValue, 1);
     }
     else if ((global_Data.DeviceType == DEVICE_TYPE_GAS) || (global_Data.DeviceType == DEVICE_TYPE_LIGHT))
     {
       uint8_t Buff[2] = {(uint8_t)(global_Data.ADCValue >> 8), (uint8_t)global_Data.ADCValue};
       SendLenth = packet_data(SendBuff, (uint16_t)global_Data.DeviceAddr, (uint8_t)global_Data.DeviceType, Buff, sizeof(Buff));
     }
     else if (global_Data.DeviceType == DEVICE_TYPE_TEMPHUMI)
     {
       uint8_t Buff[2] = {(global_Data.HumiValue), global_Data.TempValue};
       SendLenth = packet_data(SendBuff, (uint16_t)global_Data.DeviceAddr, (uint8_t)global_Data.DeviceType, Buff, sizeof(Buff));
     }
     else if (global_Data.DeviceType == DEVICE_TYPE_ULTRASONIC)
     {
       uint8_t Buff[2] = {(uint8_t)(global_Data.Distance >> 8), (uint8_t)global_Data.Distance};
       SendLenth = packet_data(SendBuff, (uint16_t)global_Data.DeviceAddr, (uint8_t)global_Data.DeviceType, Buff, sizeof(Buff));
     }
     else if (global_Data.DeviceType == DEVICE_TYPE_LAMP)
     {
       uint8_t Buff[3] = {global_Data.RedValue, global_Data.GreenValue, global_Data.BlueValue};
       SendLenth = packet_data(SendBuff, (uint16_t)global_Data.DeviceAddr, (uint8_t)global_Data.DeviceType, Buff, sizeof(Buff));
     }
      // 发送数据
      if (SendLenth != -1){
        HAL_UART_Transmit(&huart2, SendBuff, SendLenth, 100);

        HAL_UART_Transmit(&huart1, SendBuff, SendLenth, 100);

      }
    }

    if (Usart2type.UsartRecFlag)
    {
      HAL_UART_Transmit(&huart1, Usart2type.UsartRecBuffer, Usart2type.UsartRecLen, 100);

      // 解析数据包
      int ret = analysis_Packet(Usart2type.UsartRecBuffer, Usart2type.UsartRecLen);
      if (ret != 0)
        printf("result code: %d\r\n", ret);

      Usart2type.UsartRecFlag = 0; //清空接收标志位
      Usart2type.UsartRecLen = 0;  //清空接收长度
      memset(Usart2type.UsartRecBuffer, 0, sizeof(Usart2type.UsartRecBuffer));
    }
  }
}
