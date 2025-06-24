#include "mygpio.h"

void MYGPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 循迹中 	PA11*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;      // 低速（标准库最低速为 2MHz）
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_11);					  // 默认关闭 LED
	
	/* 清扫中 	PA12*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;      // 低速（标准库最低速为 2MHz）
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_12);					  // 默认关闭 LED
		
	/* 电源 	PA15*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;      // 低速（标准库最低速为 2MHz）
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_15);					  // 默认开启 LED
	
	/* 舵机 	PB1*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;      // 低速（标准库最低速为 2MHz）
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_1);					  // 默认关闭舵机
	
	/* 蜂鸣器 	C13*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;      // 低速（标准库最低速为 2MHz）
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);					  // 默认关闭蜂鸣器
	
}
