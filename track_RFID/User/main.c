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
*   连线说明：
*   1--SDA  <----->PA4	PB12
*   2--SCK  <----->PA5	PB13
*   3--MOSI <----->PA7	PB15
*   4--MISO <----->PA6  PB14    
*   5--悬空
*   6--GND <----->GND
*   7--RST <----->PB0	PA8
*   8--VCC <----->VCC
**/

void CleanOperate(void)
{
	uint8_t cnt = 10;
	uint8_t interval = 20;
	uint8_t i = 0;
	
	/* 0~60快 */
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
	/* 60~115慢 */
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
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);	/* 蜂鸣器响应 */
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

uint8_t control_switch = 1;		//0：默认情况下工作		1：不工作				
uint8_t control_mode = 0;		//0：默认情况下自动模式（摄像机）					1：自定义模式（环境参数）			
uint8_t mode = 0;				//0：循迹	1：清扫
volatile uint8_t flag_query = 0;			//0：还未收到答复		1：dirty 需要清扫		2：clean 不需要清扫		
uint8_t flag_no_scan_RFID = 0;	//0: 不用屏蔽RFID		1：需要屏蔽RFID
uint8_t flag_Buzzer = 0;

float turn_pram[2] = {0};

int main(void)
{
	int TraceDate;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
    uart_init(115200);	 	//串口1初始化为115200
    
	TIM4_Init();
	Encoder_Init();
	//OLED_Init();
	ADC1_Init();
	Track_Init();
	PID_Init();
	Motor_Init();
	Straight_Trail_param();
	MYGPIO_Init();
	RC522_Init();       //初始化射频卡模块

	printf("Start..");
	//Set_PWM(1200,1200);
    while(1)
    {	
		/* 开始工作 */
		if(control_switch)
		{
//			if(cnt_turn_pid >= 300)
//			{
//				cnt_turn_pid = 0;
//				printf("report:%.2f,%.2f\r\n",Turn_PID.Kp,Turn_PID.Kd);
//			}
			/* 循迹 */
			if(!mode)  
			{
				if(flag_Buzzer)
				{
					flag_Buzzer = 0;				/* 清扫完才置1 */
					Buzzer();
				}
				GPIO_ResetBits(GPIOA, GPIO_Pin_11);	/* 开启循迹LED */
				GPIO_SetBits(GPIOA, GPIO_Pin_12);	/* 关闭清扫LED */
				if(RC522_Handle())
				{
					mode = 1;
					Motor_Stop();
					GPIO_ResetBits(GPIOC, GPIO_Pin_13);	/* 蜂鸣器响应2s */
					delay_ms(2000);			
					GPIO_SetBits(GPIOC, GPIO_Pin_13);

				}else{
					TraceDate = GetTraceData();
					TraceMove(TraceDate, 0.05);
				}
			}else{
				/* 清扫 */
				GPIO_SetBits(GPIOA, GPIO_Pin_11);	/* 关闭循迹LED */
				GPIO_ResetBits(GPIOA, GPIO_Pin_12);	/* 开启清扫LED */
				printf("Query!!");
				while(flag_query == 0);		/* 清扫之前查询是否需要清扫 或清扫之后等待查询结果*/
				if(flag_query == 1)
				{
					GPIO_ResetBits(GPIOC, GPIO_Pin_13);	/* 蜂鸣器响应300ms */
					delay_ms(300);	
					GPIO_SetBits(GPIOC, GPIO_Pin_13);
					CleanOperate();
				}else if(flag_query == 2){
					mode = 0;				/* 清扫完毕，继续循迹 注意起步屏蔽RFID 2.5s时间 */
					flag_no_scan_RFID = 1;
					flag_Buzzer  = 1;
					GPIO_SetBits(GPIOC, GPIO_Pin_13);	/* 关闭蜂鸣器 */
				}
				flag_query = 0;
			}
		}else{
		/* 停止工作 */
			Motor_Stop();
			/* 舵机回位 */
		}
		
    }
}
