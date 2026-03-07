#ifndef _headfile_h
#define _headfile_h

#include "stm32f1xx_hal.h"
#include "stdint.h"

/***************** *** ****************/
#include "main.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"
/***************** Bsp ****************/
#include "bsp_gpio.h"
#include "bsp_iic.h"
#include "bsp_delay.h"
/***************** Modules ****************/
#include "led.h"
#include "key.h"
#include "beep.h"
#include "oled.h"
#include "mpu6050.h"
#include "motor.h"
#include "encoder.h"
#include "hcsr04.h"
#include "bt.h"
/***************** App ****************/
#include "mode.h"
#include "control.h"
#include "utils.h"
#include "ui.h"
/************************ RTOS *********************/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#endif