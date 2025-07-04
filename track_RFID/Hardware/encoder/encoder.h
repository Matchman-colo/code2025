#ifndef __ENCODER_H
#define __ENCODER_H

#include "timer.h"
#include "stm32f10x.h"

/***********************编码器参数定义**************************/
#define ACircleLeftEncode 1560	//一圈 总分辨率（13*4）*减速比（30）
#define ACircleRightEncode 1560	//一圈1560
#define ACircleEncoder ACircleLeftEncode 

#define RR 30u                // 减速比

void Encoder_Init(void);
int  GetMotorPulse(int timx);
long Nlaps_Encoder_Cnt(int num);
long Rpm_Encoder_Cnt(float rpm,uint16_t ppr,uint16_t ratio,uint16_t cnt_time);

#endif
