#include "ESP_WiFi.h"
#include "usart.h"

#include <stdio.h>
#include <string.h>
#include "lcd.h"

static tsTimeType timeESP, timeSendData;              // 软件定时器
teESP_TaskStatus ESP_TaskStatus;  		// 模组任务状态
static teATCmdNum ATNextCmdNum;         // 下条指令
static teATCmdNum ATCurrentCmdNum;      // 当前指令
static teATCmdNum ATRecCmdNum;          // 接收指令
static uint8_t CurrentRty;              // 重发次数

char *SSID = "1001";
static char* LOCAL_WIFI_IP = NULL;  // 本地IP地址


tsATCmds ATCmds[] = {
    {"AT\r\n", "OK", 500, NO_REC, 20},   																//AT指令测试
    {"AT+CWMODE=1\r\n", "OK", 500, NO_REC, 20},      										//设置为STA模式
    {"AT+CWJAP=\"farsight_%s\",\"1234567890123\"\r\n", "OK", 3000, NO_REC, 20},	//设置AP的SSID、密码
		{"AT+CIFSR\r\n", "OK", 500, NO_REC, 5}, // 查询本地IP地址
		{"AT+CIPSTART=\"UDP\",\"192.168.0.15\",8080,20000,0\r\n", "OK", 1000, NO_REC, 20}, 	//设置UDP服务端模式 
		{"AT+CIPMUX=0\r\n", "OK", 500, NO_REC, 20},													// 配置WiFi为单链接模式
    {"AT+CIPMODE=1\r\n", "OK", 500, NO_REC, 20}, 												// 设置透传模式
    {"AT+CIPSEND\r\n", "AT+CIPSEND\r\r\n\r\nOK\r\n\r\n>", 500, NO_REC, 20}, // 开启透传数据
		
    {"+++", "+++", 2000, NO_REC, 3},						// 关闭透传数据
    {"AT+CIPMODE=0\r\n", "OK", 500, NO_REC, 3},	// 关闭透传模式
    {"AT+CIPCLOSE\r\n", "OK", 500, NO_REC, 3}, 	// 断开连接
};


/**
 * 函数功能： 设置软件定时器的定时值
 * 参数：
 *     @tsTimeType *TimeType ： 软件定时器的结构体
 *     @uint32_t TimeInter ；   软件定时器的时间（单位：ms）
 * 返回值：
 *      无
*/
static void setTime(tsTimeType *TimeType, uint32_t TimeInter)
{
    TimeType->TimeStart = HAL_GetTick();    //获取系统运行时间
    TimeType->TimeInter = TimeInter;      //间隔时间
}

/**
 * 函数功能： 比较软件定时启的值
 * 参数：
 *      @tsTimeType *timeType： 软件定时器结构体
 * 返回值：
 *      返回1，代表超时
*/
static uint8_t  compareTime(tsTimeType *TimeType)  //比较时间
{
    return ((HAL_GetTick()-TimeType->TimeStart) >= TimeType->TimeInter);  //返回1代表超时
}

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
	
	char ATSendBuff[100] = {0};
	
	if(ATCmdNum == AT_CWJAP)
	{
		sprintf(ATSendBuff, ATCmds[AT_CWJAP].ATSendStr, SSID);
		HAL_UART_Transmit(&huart2, (uint8_t*)ATSendBuff, strlen(ATSendBuff), 0xff);
    printf("SendDataBuff:%s\r\n",ATSendBuff);
	}
	else
	{
		HAL_UART_Transmit(&huart2, (uint8_t*)ATCmds[ATCmdNum].ATSendStr, strlen(ATCmds[ATCmdNum].ATSendStr), 0xff);
    printf("SendDataBuff:%s\r\n",ATCmds[ATCmdNum].ATSendStr);
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
    if(Usart2type.UsartRecFlag)    // 串口接收到数据
    {
        printf("RecvStr: %s, Rev: %s\r\n",ATCmds[ATRecCmdNum].ATRecStr, Usart2type.UsartRecBuffer);

        if(strstr((const char*)Usart2type.UsartRecBuffer, ATCmds[ATRecCmdNum].ATRecStr) != NULL) //如果包含对应校验符
        {
            ATCmds[ATRecCmdNum].ATStatus = SUCCESS_REC;  //设置为成功接收
        }
				
				// 如果是查询IP地址成功了
				if ((ATRecCmdNum == AT_CIFSR) && (ATCmds[AT_CIFSR].ATStatus == SUCCESS_REC))
				{
					char *pStrstr = NULL;
					if ((pStrstr = strstr((const char *)Usart2type.UsartRecBuffer, "+CIFSR:STAIP")) != NULL)
					{
						char *pStrtok = NULL;
						pStrtok = strtok(pStrstr, "\"");
						printf("========= pStrtok: %s\r\n", pStrtok);
						
						LOCAL_WIFI_IP = strtok(NULL, "\"");
						printf("========= Local WiFi IP: %s\r\n", LOCAL_WIFI_IP);
						Gui_DrawFont_GBK16(30,48,BLACK,YELLOW,(uint8_t*)LOCAL_WIFI_IP);
						LED_NET_ON;
						
						pStrstr = NULL;
						pStrtok = NULL;
					}
				}
				
        Usart2type.UsartRecFlag = 0;  //清空接收标志位
        Usart2type.UsartRecLen = 0;   //清空接收长度
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

    if(ATCmds[ATCurrentCmdNum].ATStatus == SUCCESS_REC)         // 成功接收到数据包
    {
        switch(ATCurrentCmdNum)
        {
        case AT:			// AT指令测试成功
            printf("\r\n AT success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;

        case AT_CWMODE:			// 设置WiFi工作模式
            printf("\r\n AT_CWMODE = station_mode, success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;

				 case AT_CWJAP:		  // 连接WiFi热点
            printf("\r\n AT_CWJAP success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;
				
				case AT_CIFSR:		  // 查询本地IP地址
					printf("\r\n AT_CIFSR success...\r\n");
					
					Gui_DrawFont_GBK16(94,32,BLACK,YELLOW,(uint8_t*)SSID);
					
					ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
					ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
					ESP_TaskStatus = ESP_SEND;
					break;
						
				 case AT_CIPSTART:		//设置UDP服务端模式 
            printf("\r\n AT_CIPSTART success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
						break;
				 
        case AT_CIPMUX:			// 配置WiFi为单链接模式
            printf("\r\n AT+CIPMUX=0 success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;


        case AT_CIPMODE_1:		// 设置透传模式
            printf("\r\n AT_CIPMODE success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;

        case AT_CIPSEND:		// 发送数据成功

            printf("\r\n AT_CIPSEND success...\r\n");
            ESP_TaskStatus = ESP_IDIE;
						
						setTime(&timeSendData, 1000);
            break;


        case AT_QUITSEND:		// 结束透传数据成功

            printf("\r\n AT_QUITSEND success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;

            break;

        case AT_CIPMODE_0:		// 关闭透传模式

            printf("\r\n AT_CIPMODE_0 success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;

        case AT_CIPCLOSE:		// 关闭TCP连接

            printf("\r\n AT_CIPCLOSE success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(AT_CIPSTART);		// 开始建立TCP连接
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;

        default:
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;          // 发送下一条命令
            break;
        }
    }
    else if(compareTime(&timeESP))      //还没收到预期数据  并且超时
    {
        printf("CurrentRty: %d, CmdNum: %d\r\n",CurrentRty, ATCurrentCmdNum);
        switch(ATCurrentCmdNum)  //超时出错的命令处理
        {

        case AT:
            ATCurrentCmdNum = AT_QUITSEND;  //结束透传数据
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;          // 发送下一条命令
            break;

        default:
            break;
        }

        ATCmds[ATCurrentCmdNum].ATStatus = TIME_OUT;

        if(CurrentRty > 0)  //重发
        {
            CurrentRty--;
            printf("now:%d,trytimes:%d\r\n",ATCurrentCmdNum,CurrentRty);
            ATNextCmdNum = ATCurrentCmdNum;  //下一条 还是当前命令  实现重发效果
            ESP_TaskStatus = ESP_SEND;
            return ;
        }
        else  //重发次数用完
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

            SEND_TASK();	// 空闲状态
            return;
				
        case ESP_SEND: // 发送状态 发送ESP8266的AT指令
            if (ATCurrentCmdNum != ATNextCmdNum) // 不是重发状态
            {
                CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum; //重发次数赋值
            }

            ATSend(ATCurrentCmdNum); //发送数据
            ESP_TaskStatus = ESP_WAIT;
            return;
        case ESP_WAIT: //等待数据状态 接收ESP8266的AT指令返回的数据

            ATRecv();   // 接收数据
            Rec_WaitAT();   //处理成功命令与失败命令
            return;
        default:
						// 其他
            return;
        }
    }
}

/**
 * 函数功能： ESP8266 发送数据任务
 * 参数：
 *      无
 * 返回值：
 *      无
*/
void SEND_TASK(void)
{
    if(ESP_TaskStatus == ESP_IDIE)
    {
			if(Usart1type.UsartRecFlag)
			{
				HAL_UART_Transmit(&huart2, Usart1type.UsartRecBuffer, Usart1type.UsartRecLen, 100);
				
				LED_SEND_ON;
				HAL_Delay(50);
				
				Usart1type.UsartRecFlag = 0;  //清空接收标志位
				Usart1type.UsartRecLen = 0;   //清空接收长度
				memset(Usart1type.UsartRecBuffer, 0, sizeof(Usart1type.UsartRecBuffer));
			}
			LED_SEND_OFF;
			
			if(Usart2type.UsartRecFlag)    
			{
				HAL_UART_Transmit(&huart1, Usart2type.UsartRecBuffer, Usart2type.UsartRecLen, 100);
				
				LED_RECV_ON;
				HAL_Delay(50);
				
				Usart2type.UsartRecFlag = 0;  //清空接收标志位
				Usart2type.UsartRecLen = 0;   //清空接收长度
				memset(Usart2type.UsartRecBuffer, 0, sizeof(Usart2type.UsartRecBuffer));
			}
			LED_RECV_OFF;
    }
}






