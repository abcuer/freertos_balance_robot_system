#ifndef __UTILS_H__
#define __UTILS_H__
#include "pid.h"

typedef struct{
	uint8_t flag;
	uint16_t time;
} SoundLight_t;

void System_Init(void);
void SoundLightRun(void);
void SoundLightUpdate(void);



#endif
