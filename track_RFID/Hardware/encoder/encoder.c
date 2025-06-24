#include "encoder.h"

//void Encoder_Init(void) {
//	
//	// 开启编码器
//	HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_1); // 开启编码器A 
//  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_2); // 开启编码器B;	
//	HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_1); // 开启编码器A 
//  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_2); // 开启编码器B;	
//}

void Encoder_Init(void) 
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;

    /* 1. 使能时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 | RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* 2. 配置GPIO（TIM1_CH1/CH2: PA8/PA9；TIM3_CH1/CH2: PA6/PA7）*/
    // TIM1 GPIO配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 浮空输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // TIM3 GPIO配置
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* 3. 配置TIM1编码器模式 */
    TIM_TimeBaseStructure.TIM_Prescaler = 0;          // 预分频器
    TIM_TimeBaseStructure.TIM_Period = 65535;         // 自动重装载值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 无分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    TIM_EncoderInterfaceConfig(TIM1, 
        TIM_EncoderMode_TI12,    // 编码器模式：TI1和TI2边沿计数
        TIM_ICPolarity_Rising,   // 极性：上升沿
        TIM_ICPolarity_Rising);
    
    TIM_ICStructInit(&TIM_ICInitStructure);
    TIM_ICInitStructure.TIM_ICFilter = 0x0;           // 输入滤波
    TIM_ICInit(TIM1, &TIM_ICInitStructure);

    TIM_Cmd(TIM1, ENABLE);        // 使能TIM1

    /* 4. 配置TIM3编码器模式 */
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_Period = 65535;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_EncoderInterfaceConfig(TIM3, 
        TIM_EncoderMode_TI12, 
        TIM_ICPolarity_Rising, 
        TIM_ICPolarity_Rising);
    
    TIM_ICInitStructure.TIM_ICFilter = 0x0;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);

    TIM_Cmd(TIM3, ENABLE);        // 使能TIM3
}

/**
 * @function: int GetMotorPulse(TIM_HandleTypeDef* htim)
 * @description: 使用STM32编码器模式，读取编码器产生的脉冲值
 * @param {*} 
 * @return {*}
 */
int GetMotorPulse(int timx) {
	int encoder_counter;   //STM32编码器模式读取的总脉冲数
	switch(timx) {
		case 1: 
			encoder_counter = (short)TIM_GetCounter(TIM1); // 获取编码器数值
			TIM_SetCounter(TIM1, 0);   // 计数器清零
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
 * @function: 计算转数对应编码器脉冲数
 * @description: 使用STM32编码器模式，读取编码器产生的脉冲值
 * @param 
				[in]num: 输入小车的圈数
 * @return  
 */
long  Nlaps_Encoder_Cnt(int num) {
	return num * ACircleEncoder;         /* 4倍频 */
}

/**************************************************************************
功    能: 计算转速对应编码器脉冲数
输    入: rpm：转速；ppr：码盘数；ratio：减速比；cnt_time：计数时间(ms)
返 回 值: 电机脉冲数  转rpm所需的脉冲数
**************************************************************************/
long Rpm_Encoder_Cnt(float rpm,uint16_t ppr,uint16_t ratio,uint16_t cnt_time)
{
    return (rpm*ratio*ppr*4)/(60*1000/cnt_time);            /* 4倍频 */       
}

