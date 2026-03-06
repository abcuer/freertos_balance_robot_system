#include "headfile.h"

/* 直立环 */
/* 1. 只启用直立环kp, 调整 kp 到小车高频振荡 */
/* 2. 只启用直立环kp、ki, 调整 ki 到小车高频低幅振荡 */
/* 3. 将kp、ki都乘上 0.6 ,调整速度环 */
PIDParam_t upright_pid = {
    .kp = 360*0.6,
    .kd = -28*0.6, 
	.out = 0,
    .tar = 0.65
};

/* 速度环 */
/* 4. 只启用直立环和速度环kp, 调整 kp 到小车平稳 */
/* 5. ki = kp/200 */
PIDParam_t speed_pid = {
	.kp = 0.6,  			
	.ki = 0.6/200,		
	.out = 0,
	.filter = 0.7,
	.tar = 0  /* 前进 后退 */
};

/* 转向环 */
/* 6. 只启用转向环kd, 令 kd为10或者-10，转动其中一只轮子，另一只轮子同向转动，符号正确 */
/* 7. 同时启用三环，调整参数直到满意为止(转向环不需要大调整) */
PIDParam_t turn_pid = {
	.kd = -0.28,	/* 左右移动 */
	.kp = -20, 	/* 遥控模式下的转向速度 */
	.out = 0,
	.tar = 0
};

PIDParam_t dist_pid = {
	.kp = -0.08,				
	.ki = -0.08/200,			
	.out = 0,
	.tar = 150
};

PID_t dist;
        
// 当前测得的超声波距离值（单位：cm），用于跟随/避障控制逻辑
static float Encoder_Err, filtered_Err, last_filtered_Err, Encoder_S;

/* 机械中值，当前角度， gy*/
float AnglePidCtrl(float tar, float current, short gy)
{
	return upright_pid.kp * (tar - current) - upright_pid.kd * gy;
}

float SpeedPidCtrl(float filter, float tar)
{
	EncoderUpdate();
	Encoder_Err = (motor_left.encoder + motor_right.encoder) - tar;
	filtered_Err = (1-filter)*Encoder_Err + filter*last_filtered_Err;
	last_filtered_Err = filtered_Err;
	Encoder_S += filtered_Err;
    if (Encoder_S > 5000) Encoder_S = 5000;
    if (Encoder_S < -5000) Encoder_S = -5000;
	if(stop_flag) 
	{
		SpeedParamReset(); // 小车偏转角度过大时清零积分量，防止小车重启时乱跑
		stop_flag = 0;
	}
	float pwm_out = speed_pid.kp*filtered_Err + speed_pid.ki*Encoder_S;
	return pwm_out;
}

float TurnPidCtrl(short gz) 
{
	return turn_pid.kp * turn_pid.tar - turn_pid.kd * gz;
}

float DistPidCtrl(void)
{
	dist.target = dist_pid.tar;
	dist.now = distance;
	PID_Calculate(&dist);
	return dist.out;
}

void SpeedParamReset(void)
{
	Encoder_Err = 0, filtered_Err = 0, last_filtered_Err = 0, Encoder_S = 0;
}

void PWMLimit(float PWMA, float PWMB)
{
	if(PWMA > MAXPWM) PWMA = MAXPWM;
	else if(PWMA < -MAXPWM) PWMA = -MAXPWM;
	if(PWMB > MAXPWM) PWMB = MAXPWM;
	else if(PWMB < -MAXPWM) PWMB = -MAXPWM;
}