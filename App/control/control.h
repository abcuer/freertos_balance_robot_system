#ifndef _control_h
#define _control_h
#include "stdint.h"
#include "pid.h"

#define MAXPWM 19000

typedef struct {
    float kp, kd, ki;
    float tar, current;
	float filter;
	float out;
} PIDParam_t;  


float AnglePidCtrl(float tar, float current, short gy);
float SpeedPidCtrl(float filter, float tar);
float TurnPidCtrl(short gz);
float DistPidCtrl(void);

void PWMLimit(float PWMA, float PWMB);
void SpeedParamReset(void);

extern PIDParam_t upright_pid;
extern PIDParam_t speed_pid;
extern PIDParam_t turn_pid;
extern PIDParam_t dist_pid;
extern PID_t dist;

#endif
