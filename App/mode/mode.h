#ifndef _mode_h
#define _mode_h
#include "stdint.h"

typedef struct{
	uint8_t flag;
	uint16_t time;
} SoundLight_t;

typedef struct {
    uint8_t mode;              // 当前工作模式: 0 平衡模式，1 蓝牙遥控，2 超声波跟随
    uint8_t balance_enable;    // 平衡控制使能：1 开启，0 暂停
    uint8_t lifted_flag;       // 提起标志位：1 表示小车被提起，0 表示正常运行
    uint16_t putdown_counter;  // 放下计数器
    uint16_t lifted_counter;   // 提起计数器
} BalanceState_t;

void Mode_KeepBalance(void);
void Mode_Select(void);

void Detect_LiftState(void);
void Detect_PutDown(void);
void Detect_FallDown(void);
void Detect_ObsDist(void);

extern BalanceState_t balance;
extern volatile uint8_t obs_blocked_flag;

#endif
