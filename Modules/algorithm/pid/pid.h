#ifndef __PID_h_
#define __PID_h_
#include "stdint.h"

enum
{
  POSITION_PID = 0,  // 位置式
  DELTA_PID,         // 增量式
};

typedef struct
{
	float target;	
	float now;
	float error[3];		
	float p,i,d;
	float pout, dout, iout;
	float out;   
	
	uint32_t mode;

}PID_t;

void PIDParamInit(PID_t *pid, uint32_t mode, float p, float i, float d);
void PID_Calculate(PID_t *pid);

#endif

