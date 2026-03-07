#include "headfile.h"
#include "user_task.h"
#include "ui.h"

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
    uint8_t last_mode = 0xFF; // 记录上一次模式
    for(;;)
    {
        if (balance.mode != last_mode) 
        {
            // 模式刚刚切换，立即刷新一次全屏
            u8g2_ClearDisplay(&u8g2); 
            last_mode = balance.mode;
        }
        UI_Show(); // 绘制UI
        osDelay(Show_PERIOD);
    }
}
