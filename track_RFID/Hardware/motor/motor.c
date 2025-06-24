#include "motor.h"
#include "encoder.h"

Motor_t Motor1;
Motor_t Motor2;
Motor_t Motor;

//void Motor_Init(void) {	
//	
//	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);     // pwm ��ʱ��1ͨ��1
//	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);     // pwm ��ʱ��1ͨ��4
////	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, 720*10);    //�޸ıȽ�ֵ���޸�ռ�ձ�
////	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, 720*10);    //�޸ıȽ�ֵ���޸�ռ�ձ�
//	Set_PWM(-720*0, -720*0);
//}

void Motor_Init(void) 
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    /* 1. ʹ��ʱ�� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* 2. ����GPIO��TIM2_CH3->PA2��TIM2_CH4->PA3��*/
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;       // �����������
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 3. ����TIM2ʱ�� */
    TIM_TimeBaseStruct.TIM_Period = 7199;               // ARRֵ�����ڣ�
    TIM_TimeBaseStruct.TIM_Prescaler = 0;               // PSCֵ����Ƶϵ����
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

    /* 4. ����PWMͨ��3 */
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;      // PWMģʽ1
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;                     // ��ʼռ�ձ�0%
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High; // �ߵ�ƽ��Ч
    TIM_OC3Init(TIM2, &TIM_OCInitStruct);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

    /* 5. ����PWMͨ��4 */
    TIM_OC4Init(TIM2, &TIM_OCInitStruct);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

    /* 6. ʹ�ܶ�ʱ�� */
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
	
	
	Set_PWM(0, 0);
}


/**************************
����ֵ����
��ڲ�����int p
ȡ��ڲ����ľ���ֵ
***************************/
int gfp_abs(int p) {
	int q;
	q = p > 0? p : (-p);
	return q;
}

/**
* @brief:���õ���ٶ�PWMֵ
* @param:
* 			[in]motor_rpwm: ����PWM PWMռ�ձ�,��Χ-4500��4500������ֵֵԽ���ٶ�Խ�죬����0����ת��С��0����ת
* 			[in]motor_b: �ҵ��PWM PWMռ�ձ�,��Χ-4500��4500������ֵֵԽ���ٶ�Խ�죬����0����ת��С��0����ת
* @return:None
*/
void Set_PWM(int motor_lpwm, int motor_rpwm) {
	//��Ϊһֱ����ת�����Բ���Ҫ�仯����
//	if(motor_lpwm > 0) { IN1(1); IN2(0); }
//	else               { IN1(0); IN2(1); }
	TIM_SetCompare3(TIM2, gfp_abs(motor_lpwm));
	
//	if(motor_rpwm > 0) { IN3(0); IN4(1); }
//	else               { IN3(1); IN4(0); }
	TIM_SetCompare4(TIM2, gfp_abs(motor_rpwm));
}

void Motor_Stop(void) {
	//IN1(0); IN2(0); IN3(0); IN4(0);
	TIM_SetCompare3(TIM2, 0);
	TIM_SetCompare4(TIM2, 0);
}

	
