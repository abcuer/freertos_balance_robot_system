#include "hcsr04.h"
#include "tim.h"
#include "bsp_delay.h"

volatile uint16_t Time = 0;          // 超声波计时
float distance_buffer[FILTER_SIZE];
uint8_t filter_index = 0;
uint16_t distance = 0;

void HCSR04_Init(void)
{
	HAL_TIM_Base_Start_IT(&htim2);
}

static void HCSR04_Start(void)
{
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    delay_us(10);  
    HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
}

static float Filter_Distance(float new_value)
{
    distance_buffer[filter_index] = new_value;
    filter_index = (filter_index + 1) % FILTER_SIZE;

    float sum = 0;
    for (int i = 0; i < FILTER_SIZE; i++)
    {
        sum += distance_buffer[i];
    }
    return sum / FILTER_SIZE;
}

void HCSR04_GetValue(void)
{
    HCSR04_Start();
    delay_us(50); 
    float raw_distance = ((Time * 0.0001f) * 34000.0f) / 2.0f;
    Time = 0;
    distance = Filter_Distance(raw_distance);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        if (HAL_GPIO_ReadPin(ECHO_GPIO_Port, ECHO_Pin) == GPIO_PIN_SET)
        {
			if (Time < MAX_TIME) 	Time++;
        }
    }
}
