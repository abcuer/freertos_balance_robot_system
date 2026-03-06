#include "task.h"
#include "mode.h"
#include "mpu6050.h"

void CtrlTask(void const * argument)
{
    for(;;)
    {
        if (osSemaphoreWait(BinarySem_MPUHandle, osWaitForever) >= 0)
        {
            // 收到信号后的逻辑
            MPU_Get_Angle(&mpu);
            Balance();											        // 主控制逻辑
            CheckLiftState();  										    // 提起检测
            CheckFallDown();										    // 倒地检测
            DetectPutDown(); 		                                    // 着陆检测
            ModeSelect();  
        }	
    }					
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == MPU_INT_Pin) 
    {
        if (BinarySem_MPUHandle != NULL) {
            osSemaphoreRelease(BinarySem_MPUHandle);
        }
    }
}

