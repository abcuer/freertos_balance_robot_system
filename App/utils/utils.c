#include "headfile.h"

static SoundLight_t soundlight = {
	.flag = 0, // 声光提示激活标志：1 表示正在进行声光报警，如避障蜂鸣器提醒；0 表示无报警
	.time = 0  // 声光提示计时器：记录声光报警的持续时间，到达设定值后自动关闭蜂鸣器等提示
};

/**
 * @brief 系统初始化函数，初始化所有模块和外设
 * @param 无
 * @retval 无
 */
void System_Init(void)
{
    LedDeviceInit();
    BeepDeviceInit();
    KeyDeviceInit();
    MPU_Init();
    Motor_Init();
    Encoder_Init();
    HCSR04_Init();

    U8G2_Init(&u8g2);
    PIDParamInit(&dist, POSITION_PID, dist_pid.kp, dist_pid.ki, dist_pid.kd);
    delay_ms(20);
}

/**
 * @brief 声光提示启动函数（例如避障触发后报警）
 * @param 无
 * @retval 无
 */
void SoundLightRun(void)
{
	if(soundlight.flag == 0)
	{
        SetLedMode(LED_FOLLOW, LED_ON);
		SetBeepMode(BEEP_SYSTEM, BEEP_ON);
		soundlight.flag = 1;
	}
}

/**
 * @brief 声光提示状态更新函数，计时并关闭报警
 * @param 无
 * @retval 无
 */
void SoundLightUpdate(void)
{
    if(soundlight.flag)
    {
        soundlight.time++;

		if(soundlight.time >= 20) 
		{
            SetLedMode(LED_FOLLOW, LED_OFF);
	    	SetBeepMode(BEEP_SYSTEM, BEEP_OFF);
			soundlight.time = 0;
			soundlight.flag = 0; 
		}
        
    }
}

