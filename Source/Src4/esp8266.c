#include "esp8266.h"
#include "usart.h"  // 串口的头文件
#include "lcd.h"
uint8_t StationIPV4[14] = {0};  //存储本地IPv4

/***********************************************************************************************************
* 函数名称: void reset_rxbuffClear(void)
* 输入参数: 
*               
* 返回参数: 
*
* 功    能: --
*
* 作    者: 2017/2/22, by SongLei
* 邮    箱: 1170152143@qq.com
************************************************************************************************************/
void reset_rxbuffClear(void)
{
  Usart2type.UsartRecFlag = 0;//清零标记
  
#if (DEBUG_PRINTF == TRUE) 
  printf("WiFi:%s\n",Usart2type.UsartRecBuffer);
#endif
  
  memset(Usart2type.UsartRecBuffer,0,strlen(Usart2type.UsartRecBuffer));
  
//  NetWorkLedstatusOnOff(NetWorkState_LED4, NetWorkState_LED4_Flag);
//  NetWorkState_LED4_Flag = (~NetWorkState_LED4_Flag);
}
/*
 * 函数名：cmdAT
 * 描述  ：对WF-ESP8266模块发送AT指令
 * 输入  ：cmdData，待发送的指令
 *         expReturn1，expReturn2，expReturn3，期待的响应，为NULL表不需响应，两者为或逻辑关系
					 timeout 等待的超时时间  ms
 * 返回  : 1，指令发送成功
 *         0，指令发送失败
 * 调用  ：被外部调用
 */
_Bool cmdAT(char *cmdData,char *expReturn1,char *expReturn2,char *expReturn3, double timeout)
{
	char cmdDataTmp[40];
	sprintf(cmdDataTmp,"%s\r\n",cmdData);  //添加指令后缀\r\n
	reset_rxbuffClear(); 
	HAL_UART_Transmit(&huart2,(uint8_t *)cmdDataTmp,strlen(cmdDataTmp),0xff);	  //发送AT指令
  
	if(expReturn1==NULL && expReturn2==NULL && expReturn3==NULL)
		return 1;
	LED_3_ON;
	
	HAL_Delay(100);
  LED_3_OFF;
	uint8_t count=0;
	_Bool ok;
  for(count = 0; count < timeout/100; count++)
	{			
			if(Usart2type.UsartRecFlag)    
			{
		//		HAL_UART_Transmit(&huart1, Usart2type.UsartRecBuffer, Usart2type.UsartRecLen, 100);
				Usart2type.UsartRecBuffer[Usart2type.UsartRecLen]='\0';
				Usart2type.UsartRecLen=0;
				
				LED_2_ON;
				HAL_Delay(50);
				// 反馈结果校验是否正确
				ok = ((_Bool)strstr((const char *)Usart2type.UsartRecBuffer,expReturn1));
				if(expReturn2!=NULL)  
					ok = ( ok ||((_Bool)strstr((const char *)Usart2type.UsartRecBuffer,expReturn2)));
				if(expReturn3!=NULL)
					ok = ( ok ||((_Bool)strstr((const char *)Usart2type.UsartRecBuffer,expReturn3)));
				
				reset_rxbuffClear();
				if(ok)   // 校验为真
					return ok;
			}
			LED_2_OFF;
					
		HAL_Delay(100);
	}

	return 0;
}
 //测试AT功能函数
_Bool testAT( void )
{
	char count=0;
 
//	ESP8266_RST_H;	
  printf("\r\n测试AT启动\r\n");
	HAL_Delay ( 100 );
	while ( count < 5 )
	{
    printf("\r\n第%d次尝试连接AT\r\n", count);
		if( cmdAT ( "AT", "OK",NULL, NULL,500) )
    {
      printf("\r\nAT成功启动\r\n");
      return 1;
    }
		__HAL_UART_DISABLE(&huart2);
   __HAL_UART_DISABLE_IT(&huart2,UART_IT_IDLE); 
		HAL_Delay(500);
		 __HAL_UART_ENABLE(&huart2);
   __HAL_UART_ENABLE_IT(&huart2,UART_IT_IDLE);
		HAL_Delay(1000);
		++ count;
	}
  return 0;
}
 //设置DHCP获取
_Bool ESP8266_DHCP_CUR (void)
{
	return cmdAT( "AT+CWDHCP_CUR=1,1", "OK",NULL, NULL,500);
}
 
/*
 * 函数名：ESP8266_Net_Mode_Choose
 * 描述  ：选择WF-ESP8266模块的工作模式
 * 输入  ：enumMode，工作模式
 * 返回  : 1，选择成功
 *         0，选择失败
 * 调用  ：被外部调用
 */
_Bool ESP8266_Net_Mode_Choose ( ENUM_Net_ModeTypeDef enumMode )
{
	switch ( enumMode )
	{
		case STA:
			return cmdAT ( "AT+CWMODE=1", "OK",NULL, NULL,500); 
		
	  case AP:
		  return cmdAT ( "AT+CWMODE=2", "OK",NULL, NULL,500); 
		
		case STA_AP:
		  return cmdAT ( "AT+CWMODE=3", "OK",NULL, NULL,500); 
		
	  default:
		  return 0;
  }
	
}
 
/*
 * 函数名：ESP8266_JoinAP
 * 描述  ：WF-ESP8266模块连接外部WiFi
 * 输入  ：pSSID，WiFi名称字符串
 *       ：pPassWord，WiFi密码字符串
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 */
_Bool ESP8266_JoinAP ( char * pSSID, char * pPassWord )
{
	char cCmd [120];
 
	sprintf ( cCmd, "AT+CWJAP_DEF=\"%s\",\"%s\"", pSSID, pPassWord );
	
	return cmdAT ( cCmd, "WIFI CONNECTED","WIFI GOT IP","OK",10000);
	
}
 
/*
 * 函数名：ESP8266_Enable_MultipleId
 * 描述  ：WF-ESP8266模块启动多连接
 * 输入  ：enumEnUnvarnishTx，配置是否多连接
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
_Bool ESP8266_Enable_MultipleId ( FunctionalState enumEnUnvarnishTx )
{
	char cStr [20];
	
	sprintf ( cStr, "AT+CIPMUX=%d", ( enumEnUnvarnishTx ? 1 : 0 ) );
	
	return cmdAT ( cStr, "OK",NULL, NULL,500);
	
}
 
/*
 * 函数名：ESP8266_Link_Server
 * 描述  ：WF-ESP8266模块连接外部服务器
 * 输入  ：enumE，网络协议
 *       ：ip，服务器IP字符串
 *       ：ComNum，服务器端口字符串
 *       ：id，模块连接服务器的ID
 * 返回  : 1，连接成功
 *         0，连接失败
 * 调用  ：被外部调用
 */
_Bool ESP8266_Link_Server ( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id)
{
	char cStr [100] = { 0 }, cCmd [120];
 
  switch (  enumE )
  {
		case enumTCP:
		  sprintf ( cStr, "\"%s\",\"%s\",%s", "TCP", ip, ComNum );
		  break;
		
		case enumUDP:
		  sprintf ( cStr, "\"%s\",\"%s\",%s", "UDP", ip, ComNum );
		  break;
		
		default:
			break;
  }
 
  if ( id < 5 )
    sprintf ( cCmd, "AT+CIPSTART=%d,%s", id, cStr);
 
  else
	  sprintf ( cCmd, "AT+CIPSTART=%s", cStr );
 
	return cmdAT ( cCmd, "OK","ALREADY CONNECTED", NULL, 1000);
	
}

/*
 * 函数名：ESP8266_Read_IP
 * 描述  ：读取IP地址
 * 输入  ：无
 * 返回  : 1，读取成功
 *         0，读取失败
 * 调用  ：被外部调用
 */
_Bool ESP8266_Read_IP(void)
{
	uint8_t cCmd[120] = "AT+CIFSR\r\n";       //获取本地IP
	uint8_t i=0;
	char ipv4[40] = {0};
  char mac[40] = {0};
		reset_rxbuffClear(); 
  HAL_UART_Transmit(&huart2, cCmd, strlen((char*)cCmd), 0xffff); // 发送AT指令
  
	HAL_Delay(200);
	Usart2type.UsartRecBuffer[Usart2type.UsartRecLen]='\0';
	Usart2type.UsartRecLen=0;
	//从接收结果中取出IP
	i = sscanf((char*)Usart2type.UsartRecBuffer, "AT+CIFSR\r\n+CIFSR:STAIP,\"%s\"\r\n+CIFSR:STAMAC,\"%s\"\r\n\r\nOK", 
				ipv4, mac);
	if(i > 0)
	{
		memcpy(StationIPV4, ipv4, strlen(ipv4)-1);
		return 1;  // 成功
	}else 
		return 0;  // 失败
	
}
/*
 * 函数名：ESP8266_UnvarnishMode
 * 描述  ：配置WF-ESP8266模块进入透传模式  
 * 输入  ：无
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
_Bool ESP8266_UnvarnishMode ( void )
{
	if ( ! cmdAT ( "AT+CIPMODE=1", "OK", NULL , NULL, 500) )
		return 0;
	return 1;
}
/*
 * 函数名：ESP8266_UnvarnishSend
 * 描述  ：配置WF-ESP8266模块开启透传发送
 * 输入  ：无
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
_Bool ESP8266_UnvarnishSend ( void )
{
	return 	  cmdAT ( "AT+CIPSEND", "OK", ">" , "AT+CIPSEND\r\r\n\r\nOK\r\n\r\n>", 5000);
	
}
/*
 * 函数名：ESP8266_UnvarnishStop
 * 描述  ：配置WF-ESP8266模块 关闭透传数据 关闭透传模式
 * 输入  ：无
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
_Bool ESP8266_UnvarnishStop ( void )
{
	if ( ! cmdAT ( "+++", "+++", NULL , NULL, 2000) )  //关闭透传数据
		return 0;
	
	return 	  cmdAT ( "AT+CIPMODE=0", "OK", NULL , NULL, 500);  //关闭透传模式
	
}
/*
 * 函数名：ESP8266_Link_Close
 * 描述  ：配置WF-ESP8266模块 断开连接
 * 输入  ：无
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
_Bool ESP8266_Link_Close ( void )
{

	return 	  cmdAT ( "AT+CIPCLOSE", "OK", NULL , NULL, 500);  
	
}
/*
 * 函数名：ESP8266_CWQAP
 * 描述  ：配置WF-ESP8266模块 断开连接AP
 * 输入  ：无
 * 返回  : 1，配置成功
 *         0，配置失败
 * 调用  ：被外部调用
 */
_Bool ESP8266_CWQAP ( void )
{

	return 	  cmdAT ( "AT+CWQAP", "OK", NULL , NULL, 500);  
	
}
//恢复出厂设置
_Bool ESP8266_RESTORE ( void )
{

	return 	  cmdAT ( "AT+RESTORE", "OK", NULL , NULL, 500);  
	
}


_Bool initEsp8266(void)
{
//	Gui_DrawFont_GBK16(0,30, BLACK, YELLOW, "INIT WIFI ...");
//	printf("\r\n退出透传模式：\r\n");
//	ESP8266_UnvarnishStop();
	printf("\r\n恢复出厂设置：\r\n");
	ESP8266_RESTORE();
	
	printf("\r\n开始测试AT启动：\r\n");
	if(testAT()==0)
	{
		printf("AT连接不上！");
		return 0;
	}
//	printf("\r\n关闭TCP/UDP连接：\r\n");
//	ESP8266_Link_Close(); 
//	printf("\r\n断开连接AP：\r\n");
//  ESP8266_CWQAP();
	
	
//	printf("\r\n设置DHCP获取：\r\n");
//	_Bool dhcpreturn=ESP8266_DHCP_CUR();
//	printf("testdhcpreturn:%d",dhcpreturn);
//	if(dhcpreturn == 0)
//	{
//		printf("设置DHCP获取失败！\r\n");
//	}
	
	printf("\r\n选择WF-ESP8266模块的工作模式为STA：\r\n");
	if(ESP8266_Net_Mode_Choose(STA)==0)
	{
		printf("选择WF-ESP8266模块的工作模式失败！\r\n");
	}
	
//	Gui_DrawFont_GBK16(0,48, BLACK, YELLOW, "ADD WIFI ...");
	printf("\r\n连接WiFi：\r\n");
	if(ESP8266_JoinAP (WIFI_SSID,WIFI_PASSWORD)==0)
	{
			printf("WiFi连接不上！\r\n");
			return 0;
	}
//		HAL_Delay(2000);
//	Gui_DrawFont_GBK16(0,66, BLACK, YELLOW, "UDP Server ...");
	printf("\r\n连接外部服务器：\r\n");
	while(ESP8266_Link_Server (enumUDP,SERVER_IP,SERVER_PORT,Single_ID_0)==0)
	{
		printf("服务器连接不上！\r\n");
		HAL_Delay(1000);
	}
	
	HAL_Delay(2000);
	printf("\r\nESP8266模块启动单连接：\r\n");
	if(ESP8266_Enable_MultipleId (DISABLE)==0)
	{
		printf("ESP8266模块启动单连接失败！\r\n");
		return 0;
	}
	
	HAL_Delay(1000);
	printf("\r\n查询本地IP地址：\r\n");
	if( ESP8266_Read_IP()==0 )
	{
		printf("地址读取失败！\r\n");
		HAL_Delay(1000);
	}
//	LED_4_ON; 
	
	printf("\r\n进入透传模式：\r\n");
	if(ESP8266_UnvarnishMode()==0)
	{
		printf("进入透传模式失败！\r\n");
		return 0;
	}
	HAL_Delay(2000);
	printf ( "\r\n启动透传......\r\n" );
	if(ESP8266_UnvarnishSend()==0)
	{
		printf("启动透传模式失败！\r\n");
		return 0;
	}
	
//	Gui_DrawFont_GBK16(0,84, BLACK, YELLOW, "WIFI OK ...");
	printf( "\r\n配置 ESP8266 完毕\r\n" );

//	HAL_Delay(1000);
//	Lcd_Clear(YELLOW) ;
	
  return 1;
}
 /*
 * 函数名：ESP8266_QueryJoinedAP
 * 描述  ：WF-ESP8266模块查询已连接WiFi信息，对比是否为指定的WiFi
 * 输入  ：pSSID，WiFi名称字符串
 *      
 * 返回  : 1，当前连接WiFi与指定的相同
 *         0，不相同
 * 调用  ：被外部调用
 */
/*_Bool ESP8266_QueryJoinedAP( char * pSSID )
{
	uint8_t i = 0;
	uint8_t cCmd [] = "AT+CWJAP_DEF?\r\n";   //获得上网信息
	char ssid[40] = {0};  // wifi名
	char bssid[40] = {0}; // MAC地址
	int channel = 0; // 频道
	int rssi = 0; // 信号强度
  HAL_UART_Transmit(&huart2, cCmd, strlen((char*)cCmd), 0xffff); // 发送AT指令
	
	HAL_Delay(100);
	UsartType2.usartDMA_rxBuf[UsartType2.Usart_rx_len]='\0';
	UsartType2.Usart_rx_len=0;
	//从接收结果中取出当前wifi信息
	i = sscanf((char*)UsartType2.usartDMA_rxBuf, "AT+CWJAP_DEF=\"%s\",\"%s\",\"%d\",\"%d\"\r\n\r\nOK", 
				ssid, bssid, &channel, &rssi);
	if(i > 0)
	{
		if(strcmp(pSSID, ssid) == 0)
			return 1;
	}
	return 0;
}
*/

//————————————————
//版权声明：本文为CSDN博主「木有3」的原创文章，遵循CC 4.0 BY-SA版权协议，转载请附上原文出处链接及本声明。
//原文链接：https://blog.csdn.net/jx5631833/article/details/126371579