#include "ESP_WiFi.h"
#include "usart.h"

#include <stdio.h>
#include <string.h>
#include "lcd.h"

static tsTimeType timeESP, timeSendData;              // �����ʱ��
teESP_TaskStatus ESP_TaskStatus;  		// ģ������״̬
static teATCmdNum ATNextCmdNum;         // ����ָ��
static teATCmdNum ATCurrentCmdNum;      // ��ǰָ��
static teATCmdNum ATRecCmdNum;          // ����ָ��
static uint8_t CurrentRty;              // �ط�����

char *SSID = "1001";
static char* LOCAL_WIFI_IP = NULL;  // ����IP��ַ


tsATCmds ATCmds[] = {
    {"AT\r\n", "OK", 500, NO_REC, 20},   																//ATָ�����
    {"AT+CWMODE=1\r\n", "OK", 500, NO_REC, 20},      										//����ΪSTAģʽ
    {"AT+CWJAP=\"farsight_%s\",\"1234567890123\"\r\n", "OK", 3000, NO_REC, 20},	//����AP��SSID������
		{"AT+CIFSR\r\n", "OK", 500, NO_REC, 5}, // ��ѯ����IP��ַ
		{"AT+CIPSTART=\"UDP\",\"192.168.0.15\",8080,20000,0\r\n", "OK", 1000, NO_REC, 20}, 	//����UDP�����ģʽ 
		{"AT+CIPMUX=0\r\n", "OK", 500, NO_REC, 20},													// ����WiFiΪ������ģʽ
    {"AT+CIPMODE=1\r\n", "OK", 500, NO_REC, 20}, 												// ����͸��ģʽ
    {"AT+CIPSEND\r\n", "AT+CIPSEND\r\r\n\r\nOK\r\n\r\n>", 500, NO_REC, 20}, // ����͸������
		
    {"+++", "+++", 2000, NO_REC, 3},						// �ر�͸������
    {"AT+CIPMODE=0\r\n", "OK", 500, NO_REC, 3},	// �ر�͸��ģʽ
    {"AT+CIPCLOSE\r\n", "OK", 500, NO_REC, 3}, 	// �Ͽ�����
};


/**
 * �������ܣ� ���������ʱ���Ķ�ʱֵ
 * ������
 *     @tsTimeType *TimeType �� �����ʱ���Ľṹ��
 *     @uint32_t TimeInter ��   �����ʱ����ʱ�䣨��λ��ms��
 * ����ֵ��
 *      ��
*/
static void setTime(tsTimeType *TimeType, uint32_t TimeInter)
{
    TimeType->TimeStart = HAL_GetTick();    //��ȡϵͳ����ʱ��
    TimeType->TimeInter = TimeInter;      //���ʱ��
}

/**
 * �������ܣ� �Ƚ������ʱ����ֵ
 * ������
 *      @tsTimeType *timeType�� �����ʱ���ṹ��
 * ����ֵ��
 *      ����1������ʱ
*/
static uint8_t  compareTime(tsTimeType *TimeType)  //�Ƚ�ʱ��
{
    return ((HAL_GetTick()-TimeType->TimeStart) >= TimeType->TimeInter);  //����1����ʱ
}

/**
 * �������ܣ� ��ʼ�� ESP8266-WiFiģ�� ��ر���
 * ������
 *      ��
 * ����ֵ��
 *      ��
*/
void ESP8266_Init(void)
{
    printf("ESP8266_Init...\r\n");
	
    ESP_TaskStatus = ESP_SEND;                        //���뷢��ģʽ
    ATCurrentCmdNum = AT;                             //��ǰָ��
    ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1); //��һ��ָ��
	
}
/**
 * �������ܣ� ESP8266 ����ATָ������
 * ������
 *      @ATCmdNum�� ATָ��ö�٣���ǰ��ATָ���Ӧ
 * ����ֵ��
 *      ��
*/
static void ATSend(teATCmdNum ATCmdNum)
{
	memset(Usart2type.UsartRecBuffer, 0, USART_REC_SIE);
	ATCmds[ATCmdNum].ATStatus = NO_REC; //����Ϊ δ����
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

	//�򿪳�ʱ��ʱ��
	setTime(&timeESP, ATCmds[ATCmdNum].TimeOut);
}
/**
 * �������ܣ� ESP8266 ��������
 * ������
 *      ��
 * ����ֵ��
 *      ��
*/
static void ATRecv(void)
{
    if(Usart2type.UsartRecFlag)    // ���ڽ��յ�����
    {
        printf("RecvStr: %s, Rev: %s\r\n",ATCmds[ATRecCmdNum].ATRecStr, Usart2type.UsartRecBuffer);

        if(strstr((const char*)Usart2type.UsartRecBuffer, ATCmds[ATRecCmdNum].ATRecStr) != NULL) //���������ӦУ���
        {
            ATCmds[ATRecCmdNum].ATStatus = SUCCESS_REC;  //����Ϊ�ɹ�����
        }
				
				// ����ǲ�ѯIP��ַ�ɹ���
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
				
        Usart2type.UsartRecFlag = 0;  //��ս��ձ�־λ
        Usart2type.UsartRecLen = 0;   //��ս��ճ���
        memset(Usart2type.UsartRecBuffer, 0, sizeof(Usart2type.UsartRecBuffer));
    }
}

/**
 * �������ܣ� ESP8266 ����ɹ�������ʧ������
 * ������
 *      ��
 * ����ֵ��
 *      ��
*/
static void Rec_WaitAT(void)
{

    if(ATCmds[ATCurrentCmdNum].ATStatus == SUCCESS_REC)         // �ɹ����յ����ݰ�
    {
        switch(ATCurrentCmdNum)
        {
        case AT:			// ATָ����Գɹ�
            printf("\r\n AT success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;

        case AT_CWMODE:			// ����WiFi����ģʽ
            printf("\r\n AT_CWMODE = station_mode, success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;

				 case AT_CWJAP:		  // ����WiFi�ȵ�
            printf("\r\n AT_CWJAP success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;
				
				case AT_CIFSR:		  // ��ѯ����IP��ַ
					printf("\r\n AT_CIFSR success...\r\n");
					
					Gui_DrawFont_GBK16(94,32,BLACK,YELLOW,(uint8_t*)SSID);
					
					ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
					ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
					ESP_TaskStatus = ESP_SEND;
					break;
						
				 case AT_CIPSTART:		//����UDP�����ģʽ 
            printf("\r\n AT_CIPSTART success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
						break;
				 
        case AT_CIPMUX:			// ����WiFiΪ������ģʽ
            printf("\r\n AT+CIPMUX=0 success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;


        case AT_CIPMODE_1:		// ����͸��ģʽ
            printf("\r\n AT_CIPMODE success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;

        case AT_CIPSEND:		// �������ݳɹ�

            printf("\r\n AT_CIPSEND success...\r\n");
            ESP_TaskStatus = ESP_IDIE;
						
						setTime(&timeSendData, 1000);
            break;


        case AT_QUITSEND:		// ����͸�����ݳɹ�

            printf("\r\n AT_QUITSEND success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;

            break;

        case AT_CIPMODE_0:		// �ر�͸��ģʽ

            printf("\r\n AT_CIPMODE_0 success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;

        case AT_CIPCLOSE:		// �ر�TCP����

            printf("\r\n AT_CIPCLOSE success...\r\n");
            ATCurrentCmdNum = (teATCmdNum)(AT_CIPSTART);		// ��ʼ����TCP����
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;
            break;

        default:
            ATCurrentCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;          // ������һ������
            break;
        }
    }
    else if(compareTime(&timeESP))      //��û�յ�Ԥ������  ���ҳ�ʱ
    {
        printf("CurrentRty: %d, CmdNum: %d\r\n",CurrentRty, ATCurrentCmdNum);
        switch(ATCurrentCmdNum)  //��ʱ����������
        {

        case AT:
            ATCurrentCmdNum = AT_QUITSEND;  //����͸������
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum+1);
            ESP_TaskStatus = ESP_SEND;          // ������һ������
            break;

        default:
            break;
        }

        ATCmds[ATCurrentCmdNum].ATStatus = TIME_OUT;

        if(CurrentRty > 0)  //�ط�
        {
            CurrentRty--;
            printf("now:%d,trytimes:%d\r\n",ATCurrentCmdNum,CurrentRty);
            ATNextCmdNum = ATCurrentCmdNum;  //��һ�� ���ǵ�ǰ����  ʵ���ط�Ч��
            ESP_TaskStatus = ESP_SEND;
            return ;
        }
        else  //�ط���������
        {
            ATCurrentCmdNum = AT;
            ATNextCmdNum = (teATCmdNum)(ATCurrentCmdNum + 1);
            ESP_TaskStatus = ESP_SEND;
            return;
        }
    }
}

/**
 * �������ܣ� ESP8266 �����״̬��
 * ������
 *      ��
 * ����ֵ��
 *      ��
*/
void ESP_TASK(void)
{
    while (1)
    {
        switch (ESP_TaskStatus)
        {
        case ESP_IDIE: // ����״̬

            SEND_TASK();	// ����״̬
            return;
				
        case ESP_SEND: // ����״̬ ����ESP8266��ATָ��
            if (ATCurrentCmdNum != ATNextCmdNum) // �����ط�״̬
            {
                CurrentRty = ATCmds[ATCurrentCmdNum].RtyNum; //�ط�������ֵ
            }

            ATSend(ATCurrentCmdNum); //��������
            ESP_TaskStatus = ESP_WAIT;
            return;
        case ESP_WAIT: //�ȴ�����״̬ ����ESP8266��ATָ��ص�����

            ATRecv();   // ��������
            Rec_WaitAT();   //����ɹ�������ʧ������
            return;
        default:
						// ����
            return;
        }
    }
}

/**
 * �������ܣ� ESP8266 ������������
 * ������
 *      ��
 * ����ֵ��
 *      ��
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
				
				Usart1type.UsartRecFlag = 0;  //��ս��ձ�־λ
				Usart1type.UsartRecLen = 0;   //��ս��ճ���
				memset(Usart1type.UsartRecBuffer, 0, sizeof(Usart1type.UsartRecBuffer));
			}
			LED_SEND_OFF;
			
			if(Usart2type.UsartRecFlag)    
			{
				HAL_UART_Transmit(&huart1, Usart2type.UsartRecBuffer, Usart2type.UsartRecLen, 100);
				
				LED_RECV_ON;
				HAL_Delay(50);
				
				Usart2type.UsartRecFlag = 0;  //��ս��ձ�־λ
				Usart2type.UsartRecLen = 0;   //��ս��ճ���
				memset(Usart2type.UsartRecBuffer, 0, sizeof(Usart2type.UsartRecBuffer));
			}
			LED_RECV_OFF;
    }
}






