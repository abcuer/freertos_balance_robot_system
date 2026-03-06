#ifndef __TASK_H
#define __TASK_H
#include "cmsis_os.h"

void CtrlTask(void const * argument);

extern osSemaphoreId BinarySem_MPUHandle;

#endif