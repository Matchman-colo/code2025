//#include "sys.h"
#include "usart.h"	  
#include "pid.h"
#include "bsp_oled.h"
#include <string.h>
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/8/18
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
//#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0){};//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 
  
  
#define RX_BUF_SIZE 20   
volatile uint8_t rx_buf[RX_BUF_SIZE]; 
volatile uint8_t rx_index = 0;      

volatile uint8_t flag_cnt_revceive = 0;		/* Ϊ1ʱ�����ʱ */
//volatile uint8_t protect_data = 1;

void uart_init(u32 bound){
  //GPIO�˿�����
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | 
                          RCC_APB2Periph_GPIOB | 
                          RCC_APB2Periph_AFIO, ENABLE);
  
// 2. ����USART1������ӳ�䣨���Ĵ��룩
   GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);  // ������ӳ�䣺TX->PB6, RX->PB7
	
	
	//USART1_TX   GPIOA.9	�ض����PB6
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; //PA.9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  //USART1_RX	  GPIOA.10��ʼ��  �ض����PB7
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  //Usart1 NVIC ����
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(USART1, &USART_InitStructure); //��ʼ������1
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1 

}
        
void USART1_IRQHandler(void)
{
	uint8_t data;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        data = USART_ReceiveData(USART1);
		flag_cnt_revceive = 1;
//		/* �������ݣ��ڳ����ж�֮ǰ������� */  
//		protect_data = 1;
        rx_buf[rx_index++] = data;

		if(rx_index >= RX_BUF_SIZE)
		{
			memset((void*)rx_buf,'\0',RX_BUF_SIZE);
			rx_index = 0;
		}
    }
}

extern volatile uint8_t flag_query;
extern uint8_t control_mode;
extern uint8_t control_switch;
extern PID_t Turn_PID;
extern uint8_t mode;

char buffer[20];

void judgeRecData(void)
{	
	
	/* ֻ����С��ͣ��֮��Ž����ж� */
	if(mode)
	{
		if(strstr((char*)rx_buf, "ON"))
		{
			flag_query = 1;
		}else if(strstr((char*)rx_buf, "OFF"))
		{
			flag_query = 2;
		}
	}
	
	if(strstr((char*)rx_buf, "COM0"))
	{
		control_mode = 0;					/* ����ģʽ0 */
	}else if(strstr((char*)rx_buf, "COM1"))
	{
		control_mode = 1;					/* ����ģʽ1 */
	}else if(strstr((char*)rx_buf, "SWI0"))
	{
		control_switch = 0;					/* ֹͣ���� */
	}else if(strstr((char*)rx_buf, "SWI1"))
	{
		control_switch = 1;					/* �������� */
	}
	//protect_data = 0;

//	if(rx_buf[0] != '\0')
//	{
//		printf("ok\r\n");
//		sscanf((char*)rx_buf,"%f,%f",&Turn_PID.Kp,&Turn_PID.Kd);
//	}
}
