#include "headfile.h"
#include "stdlib.h"
#include "math.h"

static float pwm_out, PWMA, PWMB = 0;
volatile uint8_t obs_blocked_flag = 0;  // 是否被障碍物拦住

BalanceState_t balance = {
	.mode = 0,                  // 当前工作模式: 0 平衡模式，1 蓝牙遥控，2 超声波跟随
    .lifted_flag = 0, 			// 提起标志位：1 表示小车被提起，0 表示正常运行
    .putdown_counter = 0,		// 放下计数器
    .lifted_counter = 0,		// 提起计数器
    .balance_enable = 1,		// 平衡控制使能：1 开启，0 暂停
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
	else if(balance.mode == 1)
	{
		if(obs_blocked_flag) SetBeepMode(BEEP_SYSTEM, BEEP_ON);
		else SetBeepMode(BEEP_SYSTEM, BEEP_OFF);
	} 
	else SetBeepMode(BEEP_SYSTEM, BEEP_OFF);
	// 仅当模式发生变化时清除数据
    if (balance.mode != last_mode)
    {
        SpeedParamReset();
        last_mode = balance.mode;  
    }
	if(balance.mode == 0)  //平衡模式
	{
		speed_pid.kp = 0.6;
		speed_pid.ki = 0.6/200;
		SetLedMode(LED_BALANCE, LED_ON);        
	}
	else SetLedMode(LED_BALANCE, LED_OFF);  
	if(balance.mode == 1)  // 遥控模式
	{
		speed_pid.kp = 0.6;
		speed_pid.ki = 0;
		// 开启蓝牙
		BT_Start();
		SetLedMode(LED_BLUETOOTH, LED_ON); 
	}
	else SetLedMode(LED_BLUETOOTH, LED_OFF);
	if(balance.mode == 2) // 超声波跟随
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
	static uint16_t last_distance = 0;
	if (balance.balance_enable) 					// 默认平衡模式
	{
		if(balance.mode == 2)
		{
			// 只有当距离值发生变化时（说明超声波完成了一次物理测量）才更新 PID
            if(distance != last_distance)
            {
				if(distance > 0 && distance <= 300)
				{
					speed_pid.tar = DistPidCtrl(); 
				}	
				else
				{
					speed_pid.tar = 0; 
				}	
				last_distance = distance;
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
 * @brief 提起检测：检测是否被提起
 * @param 无
 * @retval 无
 * @note 根据Pitch角度和陀螺仪Y轴速度判断，触发停止控制
 */
void Detect_LiftState(void)
{
    // 拿起条件：角度大且角速度较大，持续一定时间
    if (fabs(mpu.pitch) > 40.0f && abs(mpu.gyroyReal) > 150 && motor_right.encoder > 50)
	{
		balance.lifted_counter++;
		if (balance.lifted_counter > 30)
		{
			balance.lifted_flag = 1;
			balance.balance_enable = 0;
			balance.lifted_counter = 0;
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
    if (balance.lifted_flag || stop_flag)
    {
        if (fabs(mpu.pitch) < 20 && abs(mpu.gyroyReal) < 150 && abs(motor_right.encoder) < 120)
        {
            if (balance.putdown_counter++ > 30)
            {
				balance.lifted_flag = 0;
				balance.putdown_counter = 0;
				balance.balance_enable = 1; 
				stop_flag = 0;           // 清除紧急停止标志
            }
        }
        else
        {
            balance.putdown_counter = 0;
        }
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
	if (fabs(upright_pid.tar - mpu.pitch) > 70 && stop_flag == 0)			// 倒地检测
	{
		balance.balance_enable = 0;
		MotorStop();
	}   
}
 
/**
 * @brief 蓝牙避障逻辑
 * @param 无
 * @retval 无
 * @note 距离过近则触发声光报警并禁止运动，距离恢复后解除限制
 */

void Detect_ObsDist(void)
{
	if(balance.mode == 1 || balance.mode == 2)
	{
		HCSR04_GetValue();
	}

	if(distance > 0 && distance <= 150.0f)
	{
		if(!obs_blocked_flag && distance < 70.0f) 	
		{
			obs_blocked_flag = 1;
		}		
		else if (obs_blocked_flag && distance > 100.0f)	
		{
			obs_blocked_flag = 0; 
		}	
	}
}
