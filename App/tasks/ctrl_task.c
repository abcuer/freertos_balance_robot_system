#include "headfile.h"
#include "ctrl_task.h"

#define CTRL_PERIOD 5
#define DETECT_PERIOD 40
#define Show_PERIOD 200

void StartCtrlTask(void const * argument)
{
    for(;;)
    {
        MPU_Get_Angle(&mpu, CTRL_PERIOD / 1000.0f);  // 获取当前角度
        Mode_KeepBalance();									 // 主控制逻辑
        osDelay(CTRL_PERIOD);
    }					
}

void StartDetectTask(void const * argument)
{
    BT_Init();
    for(;;)
    {
        Mode_Select();  
        Detect_LiftState();  										// 提起检测
        Detect_FallDown();										    // 倒地检测
        Detect_PutDown(); 		                                    // 着陆检测
        Detect_ObsDist();

        osDelay(DETECT_PERIOD);
    }
}

void StartOLEDTask(void const * argument)
{
    for(;;)
    {
        // 收到信号后的逻辑
	    OLED_ShowString(1, 3, "Balance_Car");
        OLED_ShowNum(2, 3, mpu.pitch, 3);
        OLED_ShowNum(2, 9, distance, 3);
        OLED_ShowNum(3, 9, balance.mode, 1);
        osDelay(Show_PERIOD);
    }
}
