#include "pid.h"
#include <stdio.h>

PID_t Location;   // 位置环
PID_t Velocity;   // 速度环

PID_t Incremental;// 增量式
PID_t Position;   // 位置式

PID_t Turn_PID;       // 转向环

void PID_Init(void) {
	
	// 位置环PID参数
	Location.Kp = 0.00f;
	Location.Ki = 0.00f;
	Location.Kd = 0.0f;
	
	
	// 速度环PID参数
	Velocity.Kp = -0.0f;
	Velocity.Ki = -0.0f;
	Velocity.Kd = -0.0f;	
}

// 直行循迹参数
void Straight_Trail_param(void) 
{
	
	Position.Kp = -30.00f;
	Position.Ki = -2.600f;
	Position.Kd = 0.0f;
	
	Turn_PID.Kp = -3.0f;
	Turn_PID.Kd = 3.0f;
}

/**
* @brief:位置环PID
* @param:
* 			[in]Location_Actual_Val: 测量实际值
* 			[in]Location_Target_Val: 给定目标值
* @return:位置环PID输出值
*/
float Location_PID_Realize(float Location_Actual_Val, float Location_Target_Val) {
	static float Location_Error = 0.0;//位置环的偏差
	static float Location_ErrorLast = 0.0;//位置环的上一次偏差
	static float Location_Integral = 0.0;//位置环的积分值
	static float Location_Out = 0.0;//输出PWM
  //1.计算偏差
	Location_Error=Location_Target_Val-Location_Actual_Val;
	
	//2.累计偏差
	Location_Integral+=Location_Error;
	
	//3.积分限幅
	if(Location_Integral>5000)Location_Integral=5000;
	if(Location_Integral<-5000)Location_Integral=-5000;
	
	//3.PID算法实现
	Location_Out=(Location.Kp*Location_Error)
								+(Location.Ki*Location_Integral)
								+(Location.Kd*(Location_Error-Location_ErrorLast));
	
	//4.偏差的传递
	Location_ErrorLast=Location_Error;
	
	//5.返回位置环计算得到的输出值
	return Location_Out;  

}

/**
* @brief:速度环PID
* @param:
* 			[in]Velocity_Actual_Val: 测量实际值
* 			[in]Velocity_Target_Val: 给定目标值
* @return:速度环PID输出值
*/
float Velocity_PID_Realize(float Velocity_Actual_Val, float Velocity_Target_Val) { 
	static float Velocity_Error = 0.0;//速度环的偏差
	static float Velocity_ErrorLast = 0.0;//速度环的上一次偏差
	static float Velocity_Integral = 0.0;//速度环
	static float Velocity_Out = 0.0;
  //1.计算偏差
	Velocity_Error=Velocity_Target_Val-Velocity_Actual_Val;
	
	Velocity_Integral+=Velocity_Error;
		//3.积分限幅
	if(Velocity_Integral>5000)Velocity_Integral=5000;
	if(Velocity_Integral<-5000)Velocity_Integral=-5000;
	//3.PID算法实现
	Velocity_Out=(Velocity.Kp*Velocity_Error)
								+(Velocity.Ki*Velocity_Integral)
								+(Velocity.Kd*(Velocity_Error-Velocity_ErrorLast));
	
	//4.偏差的传递

	
	Velocity_ErrorLast = Velocity_Error;
	
	
	//7.返回速度环计算得到的输出值
	return Velocity_Out;  
}

/**
* @brief:位置式PID
* @param:
* 			[in]reality: 测量实际值
* 			[in]target: 给定目标值
* @return:位置式PID输出值
*/
float Position_PID_Realize(float reality, float target) { 	
    static float Bias,Pwm,Last_Bias,Integral_bias=0;
    
    Bias=target-reality;                            /* 计算偏差 */
    Integral_bias+=Bias;	                        /* 偏差累积 */
    
    if(Integral_bias> 2000) Integral_bias = 2000;   /* 积分限幅 */
    if(Integral_bias<-2000) Integral_bias =-2000;
    Pwm = (Position.Kp*Bias)                        /* 比例环节 */
         +(Position.Ki*Integral_bias)               /* 积分环节 */
         +(Position.Kd*(Bias-Last_Bias));           /* 微分环节 */
    
    Last_Bias=Bias;                                 /* 保存上次偏差 */
    return Pwm;                                     /* 输出结果 */
}


/**
* @brief:增量式PID
* @param:
* 			[in]reality: 测量实际值
* 			[in]target: 给定目标值
* @return:增量式PID输出值
*/
float Incremental_PID_Realize(float reality, float target) {
	
	 static float Bias, Pwm, Last_bias = 0.0, Prev_bias=0.0;
    
	 Bias = target-reality;                                       /* 计算偏差 */
    
	 Pwm += (Incremental.Kp*(Bias-Last_bias))                     /* 比例环节 */
           +(Incremental.Ki*Bias)                               /* 积分环节 */
           +(Incremental.Kd*(Bias - 2*Last_bias + Prev_bias));  /* 微分环节 */ 
    
   Prev_bias = Last_bias;                                       /* 保存上上次偏差 */
	 Last_bias = Bias;	                                          /* 保存上一次偏差 */
    
	 return Pwm;                                                  /* 输出结果 */
}

/**
* @brief:转向环PID
* @param:
* 			[in]Bias: 本次误差
* 
* @return:转向环PID输出值
*/
float Turn_PID_Realize(float Bias) {
		static float Turn, Last_Bias = 0.0;
	  Turn = Turn_PID.Kp * Bias + Turn_PID.Kd * (Last_Bias-Bias);
		Last_Bias = Bias;
	  return Turn;
}
