#include "myadc.h"
#include "delay.h"


void ADC1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    /* 1. 使能时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
    
    /* 2. 配置模拟输入引脚 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // 模拟输入模式
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // 模拟输入模式
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 3. ADC基本参数配置 */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;       // 独立模式
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;            // 禁用扫描模式
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;      // 禁用连续转换
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   // 数据右对齐
    ADC_InitStructure.ADC_NbrOfChannel = 1;                  // 转换通道数
    ADC_Init(ADC1, &ADC_InitStructure);
    
	/* 4. 设置ADC时钟分频（PCLK2为72MHz时，分频6得12MHz） */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);  
	
    /* 4. 校准ADC */
    ADC_Cmd(ADC1, ENABLE);                   // 开启ADC
    ADC_ResetCalibration(ADC1);              // 复位校准
    while(ADC_GetResetCalibrationStatus(ADC1)); // 等待复位完成
    ADC_StartCalibration(ADC1);              // 开始校准
    while(ADC_GetCalibrationStatus(ADC1));   // 等待校准完成
    
    /* 5. 使能ADC */
    ADC_Cmd(ADC1, ENABLE);
}


//  只用通道0 1 4 5 8
/****完全轮询模式****/
uint16_t Get_Adc(uint8_t Channel)
{
    /* 1. 配置ADC通道 */
    ADC_RegularChannelConfig(ADC1, Channel, 1, ADC_SampleTime_55Cycles5);
    
    /* 2. 启动转换 */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* 3. 等待转换完成 */
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    /* 4. 清除标志位并返回结果 */
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    return ADC_GetConversionValue(ADC1);
}

uint16_t Get_Adc_Average(uint8_t ch, uint8_t times) 
{
	uint32_t temp_val=0;
	uint8_t t;
	for(t = 0; t < times; t++)
	{
		temp_val+=Get_Adc(ch);
		delay_ms(3);
	}
	return temp_val/times;
}



