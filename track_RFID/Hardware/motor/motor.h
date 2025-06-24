#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"
#include "timer.h"

/***************���ڳ��ӵĲ���****************/
#define WheelDiameter 0.065f //����С������ֱ��,6.5cm
#define CarWidth 19.4f		//����С���������Ӽ�࣬19.4cm

// ���1
#define IN1(state)  GPIO_WriteBit(GPIOA, GPIO_Pin_10, (BitAction)(state))  // 0 1��ת  1 0 ��ת
#define IN2(state)  GPIO_WriteBit(GPIOA, GPIO_Pin_11, (BitAction)(state))
// ���2
#define IN4(state)  GPIO_WriteBit(GPIOC, GPIO_Pin_15, (BitAction)(state))
#define IN3(state)  GPIO_WriteBit(GPIOC, GPIO_Pin_14, (BitAction)(state))

#define MAX_MOTOR_PWM 1800    // 720*6

//#define Motor_Speed_Max 160   // 160RPM

typedef struct _Motor {
	float speed;
	float spin_speed;
	uint16_t pwm;
	
	long Sigma_MotorPulse;
	int Uinttime_MotorPulse;
	float Distance;
	
	float targetSpeed;      //RPM

} Motor_t;

extern Motor_t Motor1;
extern Motor_t Motor2;
extern Motor_t Motor;

int gfp_abs(int p);
void Motor_Init(void);
void Set_PWM(int motor_lpwm, int motor_rpwm);
void Motor_Stop(void);

#endif
