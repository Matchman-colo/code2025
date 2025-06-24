#ifndef __MYADC_H
#define __MYADC_H

#include "stm32f10x.h"

void ADC1_Init(void);
uint16_t Get_Adc(uint8_t Channel);
uint16_t Get_Adc_Average(uint8_t ch, uint8_t times); 

#endif
