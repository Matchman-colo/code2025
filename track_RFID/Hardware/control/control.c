#include "control.h"
#include "stdio.h"
#include "usart.h"
#include <string.h>

#define pi 3.1415926
float WheelOneCircleDis = WheelDiameter*pi;//����С����������һȦ����

//int turnpwm=0,oldturnpwm,differturnpwm;
//int spdpwml=0,spdpwmr=0;
//int pwml=0,pwmr=0;
//int TraceDate;

uint32_t cnt_receive;
extern volatile uint8_t rx_buf[20];
extern volatile uint8_t flag_cnt_revceive;
extern volatile uint8_t rx_index;
//extern volatile uint8_t protect_data;

uint32_t cnt_one = 0;
uint32_t cnt_two = 0;
uint8_t  cnt_judge_uart_data = 0;
uint8_t  cnt_no_scan_RFID = 0;
uint16_t cnt_turn_pid = 0;

extern uint8_t mode;
extern uint8_t flag_one;
extern uint8_t flag_two;
extern uint8_t flag_no_scan_RFID;
extern uint8_t control_mode;
extern uint8_t control_switch;

void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		if(flag_cnt_revceive)
		{
			if(++cnt_receive >= 20)
			{
//				if(protect_data)	/* ���ݱ������У����������� */
//					return;
//				else{
					flag_cnt_revceive = 0;	/* �ȴ��´ν����ź� */
					cnt_receive = 0;
					memset((void*)rx_buf,'\0',20);
					rx_index = 0;
//				}
			}		
		}
		
		++cnt_turn_pid;
		
		/* ֻ��������ЧRFIDʶ��ʱ��������500ms��˵��������λ�ã�����һ��λ�õ�RFID��β�ѯ��ӡ�� */
		if(++cnt_one >= 50)
			flag_one = 1;
			
		if(++cnt_two >= 50)
			flag_two = 1;
		
		/* ÿ200ms�ж�һ�δ������� */
        if(++cnt_judge_uart_data >= 20)
		{
			cnt_judge_uart_data = 0;
			judgeRecData();
		}
		
		/* ��Ҫ����RFID����ʱ2.5s */
		if(flag_no_scan_RFID)
		{
			if(++cnt_no_scan_RFID >= 250)
			{
				cnt_no_scan_RFID = 0;
				flag_no_scan_RFID = 0;
			}
		}
		
		/* ����̫����CPU������ֻ��ѭ��ģʽ�²Ž��м��� */
		if(!mode)
		{
			 //��ȡ��ǰ��λʱ������
			Motor1.Uinttime_MotorPulse = -GetMotorPulse(1);  
			Motor2.Uinttime_MotorPulse = -GetMotorPulse(3);
/*			 �����ۼ������� 
			Motor1.Sigma_MotorPulse += Motor1.Uinttime_MotorPulse;
			Motor2.Sigma_MotorPulse += Motor2.Uinttime_MotorPulse;
			 ����С����ǰ��ʻ����   ��������/ÿһȦ�������� * �ܳ�  == ���� 
			Motor1.Distance = Motor1.Sigma_MotorPulse/ACircleEncoder * pi * WheelDiameter;
			Motor2.Distance = Motor2.Sigma_MotorPulse/ACircleEncoder * pi * WheelDiameter;
			 ����ȡ��������ת��Ϊrpm 
			Motor1.speed = ((float)Motor1.Uinttime_MotorPulse)/ACircleEncoder * 6000.0f;   // ��10msת��Ϊ����Ҫ����6000  ����ACircleEncoderΪ�궨�� 1560��������һȦ�������������
			Motor2.speed = ((float)Motor2.Uinttime_MotorPulse)/ACircleEncoder * 6000.0f;   // ��10msת��Ϊ����Ҫ����6000
			//����ȡ������ת��Ϊm/s
			Motor1.speed = (float)Motor1.Uinttime_MotorPulse/ACircleEncoder * pi * WheelDiameter * 100;
			Motor2.speed = (float)Motor2.Uinttime_MotorPulse/ACircleEncoder * pi * WheelDiameter * 100;
*/
		}
    }
}
	
void OLED_Proc(void) {
	
	static __IO uint32_t uwTick_OLED_Speed;
	char buf[22];
	
//	if(uwTick - uwTick_OLED_Speed < 100) return;
//	uwTick_OLED_Speed = uwTick;
	
	sprintf(buf, "1 = %d ", Get_Adc(0));
	OLED_ShowStr(0, 0, (unsigned char* )buf, 1);
	sprintf(buf, "2 = %d ",Get_Adc(1));
	OLED_ShowStr(0, 2, (unsigned char* )buf, 1);
	sprintf(buf, "3 = %d ", Get_Adc(4));
	OLED_ShowStr(0, 3, (unsigned char* )buf, 1);
	sprintf(buf, "4 = %d ", Get_Adc(5));
	OLED_ShowStr(0, 4, (unsigned char* )buf, 1);
	sprintf(buf, "5 = %d ", Get_Adc(8));
	OLED_ShowStr(0, 5, (unsigned char* )buf, 1);
}

/**
 *@brief:����ѭ��������pid����С��ת��ʹС�����ں����м�
 * @param:
 *        [in]int TraceDate: ѭ�������������ֵ
 * @return: ���ص��ڵ���ٶȵ�ת��pwm
 */
int ChangeTraceTurn(int TraceDate)
{
	int pwm=0;
	int bias;
	bias = TraceDate;
	pwm = Turn_PID_Realize( bias);
	
	if(pwm>MAX_MOTOR_PWM)		pwm=MAX_MOTOR_PWM;//�޷�
	else if(pwm<-MAX_MOTOR_PWM)  pwm=-MAX_MOTOR_PWM;
	
	return pwm;
}

/*@brief:����pid������ߵ����Ŀ���ٶ�
 * @param:
 *        [in]int EncodeSpdL: ��ǰ��������������ֵ
 *        [in]float TarSpdL:��ߵ��Ŀ���ٶ�,����ٶ�Խ1.19m/s
 * @return: ������ߵ��������pwmռ�ձ�
 */
int ChangeSpeedMotorL(int NowEncodeSpdL,float TarSpdL)
{
	int pwm=0;
	int TarEncodeSpdL;
	TarEncodeSpdL=(int)((TarSpdL*ACircleEncoder)/(WheelOneCircleDis*100));//����Ŀ���ٶ����Ŀ��������ٶ�

	pwm=Position_PID_Realize(NowEncodeSpdL, TarEncodeSpdL);
	
	if(pwm>MAX_MOTOR_PWM)		pwm=MAX_MOTOR_PWM;//�޷�
	else if(pwm<-MAX_MOTOR_PWM)  pwm=-MAX_MOTOR_PWM;
	
	return pwm;
}

/*@brief:����pid�����ұߵ����Ŀ���ٶ�
 * @param:
 *        [in]int NowEncodeSpdR: ��ǰ�ҵ������������ֵ
 *        [in]float TarSpdR:�ұߵ��Ŀ���ٶ�,�����޷�����ٶ�ԼΪ0.7m/s
 * @return: �����ұߵ��������pwmռ�ձ�
 */
int ChangeSpeedMotorR(int NowEncodeSpdR,float TarSpdR)
{
	int pwm=0;
	int TarEncodeSpdR;
	TarEncodeSpdR=(int)((TarSpdR*ACircleEncoder)/(WheelOneCircleDis*100));//����Ŀ���ٶ����Ŀ��������ٶ�
	
	pwm=Position_PID_Realize(NowEncodeSpdR, TarEncodeSpdR);
	if(pwm>MAX_MOTOR_PWM)		pwm=MAX_MOTOR_PWM;//�޷�
	else if(pwm<-MAX_MOTOR_PWM)  pwm=-MAX_MOTOR_PWM;
	return pwm;
}

/**
 *@brief: ��С������ѭ��������
 *@param: 
 *        [in]TraceDate: ѭ�������������ֵ
 *        [in]TarSpeed:ѭ����Ŀ���ٶ�,�ٶȴ�С0-0.7m/s
 *@return: ����Ŀ��㷵��1�����򷵻�0
 */
void TraceMove(int TraceDate,float TarSpeed)
{
	int turnpwm=0;
	int spdpwml=0,spdpwmr=0;
	int pwml=0,pwmr=0;
	
	turnpwm=ChangeTraceTurn(TraceDate);//ת��PID
	
	spdpwml=ChangeSpeedMotorL(Motor2.Uinttime_MotorPulse, TarSpeed);
	spdpwmr=ChangeSpeedMotorR(Motor1.Uinttime_MotorPulse, TarSpeed);

//	printf("Encode_Left:%d Encode_Right:%d\r\n",Encode_Left,Encode_Right);

	pwmr=turnpwm+spdpwmr;
	if(pwmr>MAX_MOTOR_PWM)		pwmr=MAX_MOTOR_PWM;//�޷�
	else if(pwmr<-MAX_MOTOR_PWM)  pwmr=-MAX_MOTOR_PWM;
	
	pwml=-turnpwm+spdpwml;
	if(pwml>MAX_MOTOR_PWM)		pwml=MAX_MOTOR_PWM;//�޷�
	else if(pwml<-MAX_MOTOR_PWM)  pwml=-MAX_MOTOR_PWM;
	
	Set_PWM(pwml,pwmr);
	//Set_PWM(0,2000);
}
