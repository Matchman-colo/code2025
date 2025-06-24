#include "encoder.h"

//void Encoder_Init(void) {
//	
//	// ����������
//	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_1); // ����������A 
//  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_2); // ����������B;	
//	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_1); // ����������A 
//  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_2); // ����������B;	
//}

void Encoder_Init(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;

    /* 1. ʹ��ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 2. ����GPIO��TIM1_CH1/CH2: PA8/PA9��TIM3_CH1/CH2: PA6/PA7��*/
    // TIM1 GPIO����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // ��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // TIM3 GPIO����
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 3. ����TIM1������ģʽ */
    TIM_TimeBaseStructure.TIM_Prescaler = 0;          // Ԥ��Ƶ��
    TIM_TimeBaseStructure.TIM_Period = 65535;         // �Զ���װ��ֵ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // �޷�Ƶ
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_EncoderInterfaceConfig(TIM1, 
        TIM_EncoderMode_TI12,    // ������ģʽ��TI1��TI2���ؼ���
        TIM_ICPolarity_Rising,   // ���ԣ�������
        TIM_ICPolarity_Rising);
    
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 0x0;           // �����˲�
    TIM_ICInit(TIM1, &TIM_ICInitStructure);

    TIM_Cmd(TIM1, ENABLE);        // ʹ��TIM1

    /* 4. ����TIM3������ģʽ */
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_EncoderInterfaceConfig(TIM3, 
        TIM_EncoderMode_TI12, 
        TIM_ICPolarity_Rising, 
        TIM_ICPolarity_Rising);
    
    TIM_ICInitStructure.TIM_ICFilter = 0x0;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);

    TIM_Cmd(TIM3, ENABLE);        // ʹ��TIM3
}

/**
 * @function: int GetMotorPulse(TIM_HandleTypeDef* htim)
 * @description: ʹ��STM32������ģʽ����ȡ����������������ֵ
 * @param {*} 
 * @return {*}
 */
int GetMotorPulse(int timx) {
	int encoder_counter;   //STM32������ģʽ��ȡ����������
	switch(timx) {
		case 1: 
			encoder_counter = (short)TIM_GetCounter(TIM1); // ��ȡ��������ֵ
			TIM_SetCounter(TIM1, 0);   // ����������
			break;
		case 3: 
			encoder_counter = (short)TIM_GetCounter(TIM3);
			TIM_SetCounter(TIM3, 0);			
			break;
		default: encoder_counter = 0;
	}  
  return  encoder_counter;               
}

/**
 * @function: ����ת����Ӧ������������
 * @description: ʹ��STM32������ģʽ����ȡ����������������ֵ
 * @param 
				[in]num: ����С����Ȧ��
 * @return  
 */
long  Nlaps_Encoder_Cnt(int num) {
	return num * ACircleEncoder;         /* 4��Ƶ */
}

/**************************************************************************
��    ��: ����ת�ٶ�Ӧ������������
��    ��: rpm��ת�٣�ppr����������ratio�����ٱȣ�cnt_time������ʱ��(ms)
�� �� ֵ: ���������  תrpm�����������
**************************************************************************/
long Rpm_Encoder_Cnt(float rpm,uint16_t ppr,uint16_t ratio,uint16_t cnt_time)
{
    return (rpm*ratio*ppr*4)/(60*1000/cnt_time);            /* 4��Ƶ */       
}

