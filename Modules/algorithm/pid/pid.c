#include "pid.h"
#include <math.h>

void PIDParamInit(PID_t *pid, uint32_t mode, float p, float i, float d)
{
	pid->mode = mode;
	pid->p = p;
	pid->i = i;
	pid->d = d;
}

static void PID_OutLimit(PID_t *pid)
{
    if (pid->out > MOTOR_PWM_MAX)  
    {
        pid->out = MOTOR_PWM_MAX;
    }
    else if (pid->out < -MOTOR_PWM_MAX) 
    {
        pid->out = -MOTOR_PWM_MAX;
    }
}


void PID_Calculate(PID_t *pid)
{
	// 计算当前偏差
	pid->error[0] = pid->target - pid->now;

	// PID计算
	if(pid->mode == DELTA_PID)  // 增量式
	{
		pid->pout = pid->p * (pid->error[0] - pid->error[1]);
		pid->iout = pid->i * pid->error[0];
		pid->dout = pid->d * (pid->error[0] - 2 * pid->error[1] + pid->error[2]);
		pid->out += pid->pout + pid->iout + pid->dout;
	}
	else if(pid->mode == POSITION_PID)  // 位置式
	{
		pid->pout = pid->p * pid->error[0];
		pid->iout += pid->i * pid->error[0];
		pid->dout = pid->d * (pid->error[0] - pid->error[1]);
		pid->out = pid->pout + pid->iout + pid->dout;
		
		if(fabs(pid->error[0]) < 3.0f) {
			pid->out = 0;
			pid->iout = 0; // 可选：同时清除积分项，防止小误差导致积分累积
		}
	}
	// 记录前两次的偏差
	pid->error[2] = pid->error[1];
	pid->error[1] = pid->error[0];

	PID_OutLimit(pid);
}

