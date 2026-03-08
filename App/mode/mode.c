#include "mode.h"
#include "headfile.h"
#include "stdlib.h"
#include "math.h"

static float pwm_out, PWMA, PWMB = 0;
volatile uint8_t obs_blocked_flag = 0;  // 是否被障碍物拦住

BalanceState_t balance = {
	.mode = 0,                  // 当前工作模式: 0 平衡模式，1 蓝牙遥控，2 超声波跟随
    .falldown_flag = 0, 		// 倒地标志位：1 表示小车倒地，0 表示正常运行
	.falldown_counter = 0,		// 倒地计数器
    .putdown_counter = 0,		// 放下计数器
    .balance_enable = 1,		// 平衡控制使能：1 开启，0 暂停
	.is_connected = 0,         	// 蓝牙连接状态：1已连，0断开
	.last_rx_time = 0          	// 上次接收到数据的时间戳
};

/**
 * @brief 模式选择与切换函数，根据按键切换运行模式（平衡、蓝牙、跟随）
 * @param 无
 * @retval 无
 * @note 会根据当前模式调整PID参数和控制逻辑
 */
void Mode_Select(void)
{
	static uint8_t last_mode = 0xFF;  
	if(Key_GetNum(KEY_USER))
	{
		balance.mode++;
		balance.mode %= 3;
		SetBeepMode(BEEP_SYSTEM, BEEP_ON);
	} 
	else if(balance.mode == MODE_BT_REMOTE)
	{
		if(obs_blocked_flag) SetBeepMode(BEEP_SYSTEM, BEEP_ON);
		else SetBeepMode(BEEP_SYSTEM, BEEP_OFF);
	} 
	else SetBeepMode(BEEP_SYSTEM, BEEP_OFF);
	// 仅当模式发生变化时清除数据
    if (balance.mode != last_mode)
    {
        CarParamReset();
        last_mode = balance.mode;  
    }

	if(balance.mode == MODE_BALANCE)  //平衡模式
	{
		speed_pid.kp = 0.6;
		speed_pid.ki = 0.6/200;
		SetLedMode(LED_BALANCE, LED_ON);        
	}
	else SetLedMode(LED_BALANCE, LED_OFF);  
	if(balance.mode == MODE_BT_REMOTE)  // 遥控模式
	{
		speed_pid.kp = 0.6;
		speed_pid.ki = 0;
		// 开启蓝牙处理器
		BT_Check_Connect();
		BT_Start();
		SetLedMode(LED_BLUETOOTH, LED_ON); 
	}
	else SetLedMode(LED_BLUETOOTH, LED_OFF);
	if(balance.mode == MODE_FOLLOW) // 超声波跟随
	{
		speed_pid.kp = 0.6;
		speed_pid.ki = 0;
		SetLedMode(LED_FOLLOW, LED_ON);  
	}
	else SetLedMode(LED_FOLLOW, LED_OFF);  
}

/**
 * @brief 平衡控制主函数，DMP读取姿态并计算三环PID，控制小车直立
 * @param 无
 * @retval 无
 */
void Mode_KeepBalance(void)
{
	if (balance.balance_enable) 			// 默认平衡模式
	{
		if(balance.mode == MODE_FOLLOW) 	// 超声波跟随模式
		{
			if(distance > 0.0f && distance <= 40.0f)
			{
				speed_pid.tar = DistPidCtrl(); 
			}	
			else
			{
				speed_pid.tar = 0; 
			}	
		} 
		upright_pid.out = AnglePidCtrl(upright_pid.tar, mpu.pitch, mpu.gyroyReal);
		speed_pid.out = SpeedPidCtrl(speed_pid.filter, speed_pid.tar);
		turn_pid.out = TurnPidCtrl(mpu.gyrozReal);

		pwm_out = upright_pid.out + upright_pid.kp * speed_pid.out;
		PWMA = pwm_out - turn_pid.out;
		PWMB = pwm_out + turn_pid.out;
		PWMLimit(PWMA, PWMB);
		MotorSetDuty(PWMA, PWMB);
	} 
}

/**
 * @brief 倒地检测函数
 * @param 无
 * @retval 无
 * @note 若倾斜角度超过设定阈值，则关闭平衡控制并停止电机
 */
void Detect_FallDown(void)
{
	if (fabs(upright_pid.tar - mpu.pitch) > 55.0f && stop_flag == 0)			// 倒地检测
	{
		balance.falldown_counter++;
		if (balance.falldown_counter >= 5) 
		{
			balance.balance_enable = 0;
			balance.falldown_flag = 1;
			MotorStop();
		}
	}   
}
/**
 * @brief 着陆检测：检测是否已经放下
 * @param 无
 * @retval 无
 * @note 如果Pitch角度小于阈值且静止一段时间，恢复平衡控制
 */
void Detect_PutDown(void)
{
    if (balance.falldown_counter || stop_flag)
    {
        if (fabs(mpu.pitch) < 10 && abs(mpu.gyroyReal) < 100 && abs(motor_right.encoder) < 100)
        {
            if (balance.putdown_counter++ > 20)
            {
				stop_flag = 0;  
				balance.falldown_flag = 0;
				balance.putdown_counter = 0;
				balance.falldown_counter = 0;
				balance.balance_enable = 1; 
				CarParamReset();       
            }
        }
        else
        {
            balance.putdown_counter = 0;
        }
    }
}

/**
 * @brief 检测距离逻辑
 * @param 无
 * @retval 无
 * @note 距离过近则触发声光报警并禁止运动，距离恢复后解除限制
 */

void Detect_ObsDist(void)
{
// 只有在蓝牙模式或跟随模式才进行测距（节省CPU和超声波功耗）
    if(balance.mode != MODE_BT_REMOTE && balance.mode != MODE_FOLLOW) {
        return; 
    }

	HCSR04_GetDist();
	if(balance.mode == MODE_BT_REMOTE)
	{
		if(distance <= dist_pid.flollow_range)
		{
			if(!obs_blocked_flag && distance < DANGER_DIST) 	
			{
				obs_blocked_flag = 1;
			}		
			else if (obs_blocked_flag && distance > DANGER_DIST)	
			{
				obs_blocked_flag = 0; 
			}	
		}
	}
}
