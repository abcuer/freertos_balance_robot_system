#include "headfile.h"

RX_DATA_t rx_data = {0};
static BT_Command_t bt_cmd = {0};      // 蓝牙命令结构体

osSemaphoreId binSem_UART2Handle;    // 蓝牙接收信号量

void BT_Init(void)
{
	HAL_UART_Receive_IT(&huart2, &rx_data.data, 1);
}

/**
 * @brief 蓝牙遥控模式控制逻辑
 * @param 无
 * @retval 无
 * @note 根据前后左右命令调整目标速度与转向速度
 */
static void BT_UpdateData(void)
{
    taskENTER_CRITICAL();
    if(bt_cmd.forward && bt_cmd.left)        { speed_pid.tar = MAX_Speed-7;  turn_pid.tar = MAX_Turn-5; }
    else if(bt_cmd.forward && bt_cmd.right)  { speed_pid.tar = MAX_Speed+7;  turn_pid.tar = -MAX_Turn+5;  }
    else if(bt_cmd.backward && bt_cmd.left)  { speed_pid.tar = -MAX_Speed+7; turn_pid.tar = -MAX_Turn+5; }
    else if(bt_cmd.backward && bt_cmd.right) { speed_pid.tar = -MAX_Speed-7; turn_pid.tar = MAX_Turn-5;  }

    else if(bt_cmd.forward)  { speed_pid.tar = MAX_Speed+3;  turn_pid.tar = 0; }
    else if(bt_cmd.backward) { speed_pid.tar = -MAX_Speed-3; turn_pid.tar = 0; }
    else if(bt_cmd.left)     { speed_pid.tar = 0;          turn_pid.tar = -MAX_Turn-5; }
    else if(bt_cmd.right)    { speed_pid.tar = 0;          turn_pid.tar = MAX_Turn;  }
    else 
    {
        speed_pid.tar = 0;
        turn_pid.tar = 0;
    }
    taskEXIT_CRITICAL();
}

/**
 * @brief 解析蓝牙原始字节码
 * @param data 从串口接收到的 1 字节数据
 */
static void BT_ParseData(uint8_t data)
{
    switch(data)
    {
        case 0x00: bt_cmd.forward=0; bt_cmd.backward=0; bt_cmd.left=0; bt_cmd.right=0; break; // 停止
        case 0x01: bt_cmd.forward=1; bt_cmd.backward=0; bt_cmd.left=0; bt_cmd.right=0; break; // 前
        case 0x05: bt_cmd.forward=0; bt_cmd.backward=1; bt_cmd.left=0; bt_cmd.right=0; break; // 后
        case 0x03: bt_cmd.forward=0; bt_cmd.backward=0; bt_cmd.left=1; bt_cmd.right=0; break; // 左
        case 0x07: bt_cmd.forward=0; bt_cmd.backward=0; bt_cmd.left=0; bt_cmd.right=1; break; // 右
        case 0x08: bt_cmd.forward=1; bt_cmd.backward=0; bt_cmd.left=1; bt_cmd.right=0; break; // 左前
        case 0x02: bt_cmd.forward=1; bt_cmd.backward=0; bt_cmd.left=0; bt_cmd.right=1; break; // 右前
        case 0x06: bt_cmd.forward=0; bt_cmd.backward=1; bt_cmd.left=1; bt_cmd.right=0; break; // 左后
        case 0x04: bt_cmd.forward=0; bt_cmd.backward=1; bt_cmd.left=0; bt_cmd.right=1; break; // 右后
        default:   
            // 收到未知指令时，保持上一状态或停止，这里选择停止以保安全
            bt_cmd.forward=0; bt_cmd.backward=0; bt_cmd.left=0; bt_cmd.right=0; 
            break;
    }
}

void BT_Start(void)
{
    if(osSemaphoreWait(binSem_UART2Handle, 0) == osOK)
    {
        BT_ParseData(rx_data.data);
        BT_UpdateData(); 
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        // 增加非空判断
        if(binSem_UART2Handle != NULL)
        {
            // 释放信号量，通知任务处理新数据
            osSemaphoreRelease(binSem_UART2Handle);
        }
        HAL_UART_Receive_IT(&huart2, &rx_data.data, 1);
    }
}