#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "rc522.h"
#include "timer.h"
#include "mygpio.h"
#include "control.h"
#include "track.h"
#include "bsp_oled.h"
#include "myadc.h"

/**
*   ����˵����
*   1--SDA  <----->PA4	PB12
*   2--SCK  <----->PA5	PB13
*   3--MOSI <----->PA7	PB15
*   4--MISO <----->PA6  PB14    
*   5--����
*   6--GND <----->GND
*   7--RST <----->PB0	PA8
*   8--VCC <----->VCC
**/

void CleanOperate(void)
{
	uint8_t cnt = 10;
	uint8_t interval = 20;
	uint8_t i = 0;
	
	/* 0~60�� */
	for(i = 0; i <= 33; i++)
	{
		cnt = 3;
		while(cnt--)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_1);
			delay_us(500 + i * interval);
			GPIO_ResetBits(GPIOB, GPIO_Pin_1);
			delay_us(2500 - i * interval);
			delay_ms(17);
		}
	}
	/* 60~115�� */
	for(i = 0; i <= 32; i++)
	{
		cnt = 15;
		while(cnt--)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_1);
			delay_us(1160 + i * interval);
			GPIO_ResetBits(GPIOB, GPIO_Pin_1);
			delay_us(1840 - i * interval);
			delay_ms(17);
		}
	}
	
	/* 115 ~ 60 */
	for(i = 0; i <= 32; i++)
	{
		cnt = 15;
		while(cnt--)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_1);
			delay_us(1800 - i * interval);
			GPIO_ResetBits(GPIOB, GPIO_Pin_1);
			delay_us(1200 + i * interval);
			delay_ms(17);
		}
	}
	
	/* 60 ~0 */
	for(i = 0; i <= 33; i++)
	{
		cnt = 5;
		while(cnt--)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_1);
			delay_us(1160 - i * interval);
			GPIO_ResetBits(GPIOB, GPIO_Pin_1);
			delay_us(1840 + i * interval);
			delay_ms(17);
		}
	}
}

void Buzzer(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);	/* ��������Ӧ */
	delay_ms(300);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
	delay_ms(200);
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	delay_ms(300);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
	delay_ms(200);
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	delay_ms(300);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
	delay_ms(200);
}

//extern uint16_t cnt_turn_pid;
//extern PID_t Turn_PID;

uint8_t control_switch = 1;		//0��Ĭ������¹���		1��������				
uint8_t control_mode = 0;		//0��Ĭ��������Զ�ģʽ���������					1���Զ���ģʽ������������			
uint8_t mode = 0;				//0��ѭ��	1����ɨ
volatile uint8_t flag_query = 0;			//0����δ�յ���		1��dirty ��Ҫ��ɨ		2��clean ����Ҫ��ɨ		
uint8_t flag_no_scan_RFID = 0;	//0: ��������RFID		1����Ҫ����RFID
uint8_t flag_Buzzer = 0;

float turn_pram[2] = {0};

int main(void)
{
	int TraceDate;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
    uart_init(115200);	 	//����1��ʼ��Ϊ115200
    
	TIM4_Init();
	Encoder_Init();
	//OLED_Init();
	ADC1_Init();
	Track_Init();
	PID_Init();
	Motor_Init();
	Straight_Trail_param();
	MYGPIO_Init();
	RC522_Init();       //��ʼ����Ƶ��ģ��

	printf("Start..");
	//Set_PWM(1200,1200);
    while(1)
    {	
		/* ��ʼ���� */
		if(control_switch)
		{
//			if(cnt_turn_pid >= 300)
//			{
//				cnt_turn_pid = 0;
//				printf("report:%.2f,%.2f\r\n",Turn_PID.Kp,Turn_PID.Kd);
//			}
			/* ѭ�� */
			if(!mode)  
			{
				if(flag_Buzzer)
				{
					flag_Buzzer = 0;				/* ��ɨ�����1 */
					Buzzer();
				}
				GPIO_ResetBits(GPIOA, GPIO_Pin_11);	/* ����ѭ��LED */
				GPIO_SetBits(GPIOA, GPIO_Pin_12);	/* �ر���ɨLED */
				if(RC522_Handle())
				{
					mode = 1;
					Motor_Stop();
					GPIO_ResetBits(GPIOC, GPIO_Pin_13);	/* ��������Ӧ2s */
					delay_ms(2000);			
					GPIO_SetBits(GPIOC, GPIO_Pin_13);

				}else{
					TraceDate = GetTraceData();
					TraceMove(TraceDate, 0.05);
				}
			}else{
				/* ��ɨ */
				GPIO_SetBits(GPIOA, GPIO_Pin_11);	/* �ر�ѭ��LED */
				GPIO_ResetBits(GPIOA, GPIO_Pin_12);	/* ������ɨLED */
				printf("Query!!");
				while(flag_query == 0);		/* ��ɨ֮ǰ��ѯ�Ƿ���Ҫ��ɨ ����ɨ֮��ȴ���ѯ���*/
				if(flag_query == 1)
				{
					GPIO_ResetBits(GPIOC, GPIO_Pin_13);	/* ��������Ӧ300ms */
					delay_ms(300);	
					GPIO_SetBits(GPIOC, GPIO_Pin_13);
					CleanOperate();
				}else if(flag_query == 2){
					mode = 0;				/* ��ɨ��ϣ�����ѭ�� ע��������RFID 2.5sʱ�� */
					flag_no_scan_RFID = 1;
					flag_Buzzer  = 1;
					GPIO_SetBits(GPIOC, GPIO_Pin_13);	/* �رշ����� */
				}
				flag_query = 0;
			}
		}else{
		/* ֹͣ���� */
			Motor_Stop();
			/* �����λ */
		}
		
    }
}
