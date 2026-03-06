#include "encoder.h"

void Encoder_Init(void)
{
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_1); // 开启编码器A
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_2); 	
    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_1); // 开启编码器B	
    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_2); 
}

// 左轮编码器（TIM4）
static int16_t EncoderLeftGet(void)
{
    int16_t temp;
    temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);   // 读取计数
    __HAL_TIM_SET_COUNTER(&htim4, 0);                // 清零
    return temp;
}

// 右轮编码器（TIM1）
static int16_t EncoderRightGet(void)
{
    int16_t temp;
    temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim1);
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    return -temp;
}

void EncoderUpdate(void)
{
    motor_left.encoder  = EncoderLeftGet();
    motor_right.encoder = EncoderRightGet();
}