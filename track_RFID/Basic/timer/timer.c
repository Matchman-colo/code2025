#include "timer.h"
#include "stm32f10x.h"
#include <string.h>




void TIM4_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;


    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);


    TIM_TimeBaseStructure.TIM_Period = 10000 - 1;          /* 10ms */  
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;          
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;           
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);


    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);


    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_Cmd(TIM4, ENABLE);
}


