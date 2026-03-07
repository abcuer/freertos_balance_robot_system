#ifndef __OLED_H
#define __OLED_H

#include "stdint.h"
#include "bsp_iic.h"
#include "bsp_delay.h"
#include "oled_font.h"
#include "u8g2.h"

// OLED I2C地址（7位地址，需要左移1位）
#define OLED_ADDR    0x78    // 0x3C左移1位
#define OLED_CMD_MODE    0x00    // 命令模式
#define OLED_DATA_MODE   0x40    // 数据模式

// 函数声明
void OLED_Init(void);
void OLED_Clear(void);
void U8G2_Init(u8g2_t *u8g2);

extern u8g2_t u8g2;

#endif