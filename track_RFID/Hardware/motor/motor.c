#include "motor.h"
#include "encoder.h"

Motor_t Motor1;
Motor_t Motor2;
Motor_t Motor;

//void Motor_Init(void) {	
//	
//	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);     // pwm 定时器1通道1
//	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);     // pwm 定时器1通道4
////	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_3, 720*10);    //修改比较值，修改占空比
////	__HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_4, 720*10);    //修改比较值，修改占空比
//	Set_PWM(-720*0, -720*0);
//}

void Motor_Init(void) 
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;

    /* 1. 使能时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* 2. 配置GPIO（TIM2_CH3->PA2，TIM2_CH4->PA3）*/
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;       // 复用推挽输出
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* 3. 配置TIM2时基 */
    TIM_TimeBaseStruct.TIM_Period = 7199;               // ARR值（周期）
    TIM_TimeBaseStruct.TIM_Prescaler = 0;               // PSC值（分频系数）
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);

    /* 4. 配置PWM通道3 */
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;      // PWM模式1
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;                     // 初始占空比0%
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High; // 高电平有效
    TIM_OC3Init(TIM2, &TIM_OCInitStruct);
    TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

    /* 5. 配置PWM通道4 */
    TIM_OC4Init(TIM2, &TIM_OCInitStruct);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

    /* 6. 使能定时器 */
    TIM_ARRPreloadConfig(TIM2, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
	
	
	Set_PWM(0, 0);
}


/**************************
绝对值函数
入口参数：int p
取入口参数的绝对值
***************************/
int gfp_abs(int p) {
	int q;
	q = p > 0? p : (-p);
	return q;
}

/**
* @brief:设置电机速度PWM值
* @param:
* 			[in]motor_rpwm: 左电机PWM PWM占空比,范围-4500到4500，绝对值值越大速度越快，大于0正向转，小于0逆向转
* 			[in]motor_b: 右电机PWM PWM占空比,范围-4500到4500，绝对值值越大速度越快，大于0正向转，小于0逆向转
* @return:None
*/
void Set_PWM(int motor_lpwm, int motor_rpwm) {
	//因为一直是正转，所以不需要变化方向
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

	
