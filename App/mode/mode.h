#ifndef _mode_h
#define _mode_h
#include "stdint.h"

#define DANGER_DIST 7.0f
typedef enum {
    MODE_BALANCE = 0,   // 平衡模式
    MODE_BT_REMOTE,     // 蓝牙遥控模式
    MODE_FOLLOW        // 超声波跟随模式
} WorkMode_e;

typedef struct {
    uint8_t mode;              // 当前工作模式: 0 平衡模式，1 蓝牙遥控，2 超声波跟随
    uint8_t balance_enable;    // 平衡控制使能：1 开启，0 暂停
    uint8_t falldown_flag;       // 倒地标志位：1 表示小车倒地，0 表示正常运行
    uint16_t putdown_counter;  // 放下计数器
    uint16_t falldown_counter;   // 倒地计数器
    uint8_t is_connected;      // 蓝牙连接状态：1已连，0断开
    uint32_t last_rx_time;     // 上次接收到数据的时间戳
} BalanceState_t;

void Mode_KeepBalance(void);
void Mode_Select(void);

void Detect_PutDown(void);
void Detect_FallDown(void);
void Detect_ObsDist(void);

extern BalanceState_t balance;
extern volatile uint8_t obs_blocked_flag;

#endif
