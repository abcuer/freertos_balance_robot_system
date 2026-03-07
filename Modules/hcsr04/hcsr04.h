#ifndef __HCSR04_H
#define __HCSR04_H
#include "stdint.h"

#define FILTER_SIZE 5
#define MAX_TIME 30000 

void HCSR04_Init(void);
void HCSR04_GetDist(void);

extern uint16_t distance;

#endif