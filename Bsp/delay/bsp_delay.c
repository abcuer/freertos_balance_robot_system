#include "bsp_delay.h"
#include "stm32f1xx_hal.h"

void delay_us(uint32_t us)
{
    uint32_t ticks = us * SYS_CLK / 5; 
    while (ticks--) 
    {
        __NOP(); 
    }
}

void delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}