#include "bsp_dht11.h"
#include "bsp_delay.h"


#define DHT11_GPIO_PORT 		GPIOB
#define DHT11_GPIO_PIN 			GPIO_PIN_8
#define	DHT11_DQ_IN  		    HAL_GPIO_ReadPin(DHT11_GPIO_PORT, DHT11_GPIO_PIN)

static void DHT11_IO_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

static void DHT11_IO_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

//��λDHT11
void DHT11_Rst(void)
{
    DHT11_IO_OUT(); 	//SET OUTPUT
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_RESET); 	//����DQ
    HAL_Delay(20);    	//��������18ms
    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);		//DQ=1
    delay_us(30);     	//��������20~40us
}

//�ȴ�DHT11�Ļ�Ӧ
//����1:δ��⵽DHT11�Ĵ���
//����0:����
uint8_t DHT11_Check(void)
{
    uint8_t retry=0;
    DHT11_IO_IN();//SET INPUT
    while (DHT11_DQ_IN&&retry<100)//DHT11������40~80us
    {
        retry++;
        delay_us(1);
    };
    if(retry>=100)return 1;
    else retry=0;
    while (!DHT11_DQ_IN&&retry<100)//DHT11���ͺ���ٴ�����40~80us
    {
        retry++;
        delay_us(1);
    };
    if(retry>=100)return 1;
    return 0;
}

//��DHT11��ȡһ��λ
//����ֵ��1/0
uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry=0;
    while(DHT11_DQ_IN&&retry<100)//�ȴ���Ϊ�͵�ƽ
    {
        retry++;
        delay_us(1);
    }
    retry=0;
    while(!DHT11_DQ_IN&&retry<100)//�ȴ���ߵ�ƽ
    {
        retry++;
        delay_us(1);
    }
    delay_us(40);//�ȴ�40us
    if(DHT11_DQ_IN)return 1;
    else return 0;
}

//��DHT11��ȡһ���ֽ�
//����ֵ������������
uint8_t DHT11_Read_Byte(void)
{
    uint8_t i,dat;
    dat=0;
    for (i=0; i<8; i++)
    {
        dat<<=1;
        dat|=DHT11_Read_Bit();
    }
    return dat;
}

//��DHT11��ȡһ������
//temp:�¶�ֵ(��Χ:0~50��)
//humi:ʪ��ֵ(��Χ:20%~90%)
//����ֵ��HAL_OK,����;1,��ȡʧ��
uint8_t DHT11_Read_Data(uint8_t *humiH,uint8_t *humiL,uint8_t *tempH,uint8_t *tempL)
{
    uint8_t buf[5];
    uint8_t i;
    DHT11_Rst();
    if(DHT11_Check()==0)
    {
        for(i=0; i<5; i++) //��ȡ40λ����
        {
            buf[i]=DHT11_Read_Byte();
        }
        if((buf[0]+buf[1]+buf[2]+buf[3])==buf[4])
        {
            *humiH=buf[0];
            *humiL=buf[1];
            *tempH=buf[2];
            *tempL=buf[3];

        }
    } else
        return HAL_ERROR;

    return HAL_OK;
}


//��ʼ��DHT11��IO�� DQ ͬʱ���DHT11�Ĵ���
//����1:������
//����0:����
uint8_t FS_DHT11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(DHT11_GPIO_PORT, DHT11_GPIO_PIN, GPIO_PIN_SET);	// �����
    DHT11_Rst();  //��λDHT11
    return DHT11_Check();//�ȴ�DHT11�Ļ�Ӧ
}

