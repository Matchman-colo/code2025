#ifndef __DELAY_h
#define __DELAY_h
#include "stm32f10x.h"                  // Device header

void delay_us(uint16_t us);
void delay_ms(uint16_t ms);
void delay_s(uint16_t s);

#endif
