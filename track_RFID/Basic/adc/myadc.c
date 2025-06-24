#include "myadc.h"
#include "delay.h"


void ADC1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    /* 1. ʹ��ʱ�� */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_ADC1, ENABLE);
    
    /* 2. ����ģ���������� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // ģ������ģʽ
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;  // ģ������ģʽ
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    /* 3. ADC������������ */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;       // ����ģʽ
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;            // ����ɨ��ģʽ
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;      // ��������ת��
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // �������
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;   // �����Ҷ���
    ADC_InitStructure.ADC_NbrOfChannel = 1;                  // ת��ͨ����
    ADC_Init(ADC1, &ADC_InitStructure);
    
	/* 4. ����ADCʱ�ӷ�Ƶ��PCLK2Ϊ72MHzʱ����Ƶ6��12MHz�� */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);  
	
    /* 4. У׼ADC */
    ADC_Cmd(ADC1, ENABLE);                   // ����ADC
    ADC_ResetCalibration(ADC1);              // ��λУ׼
    while(ADC_GetResetCalibrationStatus(ADC1)); // �ȴ���λ���
    ADC_StartCalibration(ADC1);              // ��ʼУ׼
    while(ADC_GetCalibrationStatus(ADC1));   // �ȴ�У׼���
    
    /* 5. ʹ��ADC */
    ADC_Cmd(ADC1, ENABLE);
}


//  ֻ��ͨ��0 1 4 5 8
/****��ȫ��ѯģʽ****/
uint16_t Get_Adc(uint8_t Channel)
{
    /* 1. ����ADCͨ�� */
    ADC_RegularChannelConfig(ADC1, Channel, 1, ADC_SampleTime_55Cycles5);
    
    /* 2. ����ת�� */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* 3. �ȴ�ת����� */
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
    
    /* 4. �����־λ�����ؽ�� */
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



