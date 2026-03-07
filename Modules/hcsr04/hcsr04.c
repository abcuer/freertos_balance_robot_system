#include "hcsr04.h"
#include "math.h"
#include "memory.h"
#include "bsp_delay.h"
#include "tim.h"
#include "cmsis_os.h"

// 引入echo信号量
extern osSemaphoreId echoSemHandle; 

volatile uint16_t start_time = 0;
volatile uint16_t pulse_width = 0;

float distance_buffer[FILTER_SIZE];
uint16_t distance = 0;

void HCSR04_Init(void)
{
    // 启动定时器作为微秒级“秒表” 
    HAL_TIM_Base_Start(&htim2); 
    for(int i=0; i<FILTER_SIZE; i++) distance_buffer[i] = 0;
}

/**
 * @brief 触发超声波测距
 */
static void HCSR04_Start(void)
{
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    // RTOS 下极短的死等延时，10us 不会严重阻塞系统
    delay_us(10);
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
}

static float HCSR04_Filter(float new_val)
{
    static float last_val = 0;
    static float history[FILTER_SIZE] = {0}; // 历史记录
    static uint8_t filter_index = 0;
    
    // --- 1. 物理范围剔除 ---
    if (new_val <= 2.0f || new_val >= 400.0f) return last_val; 

    // --- 2. 差值限幅 (Step Limit) ---
    // 如果偏差超过50cm，判定为突变。采用步进追随，不直接采用新值
    if (last_val != 0 && fabs(new_val - last_val) > 50.0f) {
        new_val = last_val + (new_val > last_val ? 15.0f : -15.0f);
    }

    // 更新历史缓冲区
    history[filter_index] = new_val;
    filter_index = (filter_index + 1) % FILTER_SIZE;

    // --- 3. 冒泡排序求中值 (剔除最大最小尖峰) ---
    float temp_list[FILTER_SIZE];
    memcpy(temp_list, history, sizeof(history)); // 拷贝一份避免破坏原始顺序
    
    for (int i = 0; i < FILTER_SIZE - 1; i++) {
        for (int j = 0; j < FILTER_SIZE - i - 1; j++) {
            if (temp_list[j] > temp_list[j + 1]) {
                float tmp = temp_list[j];
                temp_list[j] = temp_list[j + 1];
                temp_list[j + 1] = tmp;
            }
        }
    }

    // --- 4. 权重移动平均 ---
    // 此时 temp_list 是排好序的，我们取中间三位进行加权
    // 权重分配：中间值(50%)，两边值(各25%)
    float filtered_val = temp_list[1] * 0.25f + temp_list[2] * 0.50f + temp_list[3] * 0.25f;

    // --- 5. 一阶互补低通 (进一步平滑抖动) ---
    // alpha 越小，越平滑但响应越慢。0.8 是动态平衡点
    float alpha = 0.8f;
    float final_val = alpha * last_val + (1.0f - alpha) * filtered_val;

    last_val = final_val;
    return final_val;
}

void HCSR04_GetDist(void)
{
    // 1. 发射超声波触发脉冲
    HCSR04_Start();
    
    // 2. 任务挂起，等待 EXTI 中断释放信号量。超时设为 30ms。
    // 超声波测距 4 米往返约需 23.2ms，所以 30ms 等不到说明前方无遮挡或模块故障
    if (osSemaphoreWait(echoSemHandle, 30) == osOK)
    {
        // 3. 收到回波，计算距离
        // pulse_width 单位是微秒 (us)。公式：距离(cm) = 时间(us) * 0.017
        float raw_distance = pulse_width * 0.017f;
        
        // 4. 经过高级滤波获取最终距离
        distance = (uint16_t)HCSR04_Filter(raw_distance);
    }
}

/**
 * @brief EXTI 双边沿中断回调函数
 * @note 当 ECHO 引脚电平发生跳变时自动进入
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == ECHO_Pin)
    {
        // 如果是高电平，说明是上升沿触发
        if (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET)
        {
            // 记录起始时刻的定时器计数值
            start_time = __HAL_TIM_GET_COUNTER(&htim2);
        }
        else // 如果是低电平，说明是下降沿触发，回波结束
        {
            // 记录结束时刻的定时器计数值
            uint16_t end_time = __HAL_TIM_GET_COUNTER(&htim2);
            
            // 无符号 16 位整数减法，自动完美处理定时器溢出翻转 (65535 -> 0) 的问题
            pulse_width = (uint16_t)(end_time - start_time); 
            
            // 释放信号量，唤醒处于阻塞状态的 DetectTask
            if(echoSemHandle != NULL) {
                // 注意：在中断中必须调用带 FromISR 结尾的 FreeRTOS API，
                // 如果使用 CMSIS-OS V1 封装，osSemaphoreRelease 内部已处理 ISR 情况
                osSemaphoreRelease(echoSemHandle);
            }
        }
    }
}