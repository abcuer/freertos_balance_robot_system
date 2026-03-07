#include "key.h"
#include "bsp_gpio.h"
#include "gpio.h"

static KEYInstance key[KEY_NUM];

/**
 * @brief 获取原始物理电平对应的按键状态
 */
static KEY_State_e GetKeyState(KEY_Type_e KeyType)
{
    KEYInstance *ins = &key[KeyType];
    GPIO_PinState pinState = HAL_GPIO_ReadPin(ins->StaticParam.GPIO_Port, ins->StaticParam.GPIO_Pin);
    
    // 如果物理电平等于有效电平，则判定为按下
    return (pinState == (GPIO_PinState)ins->StaticParam.PressLevel) ? KEY_PRESSED : KEY_RELEASED;
}

/**
 * @brief 内部初始化函数
 */
static void Key_Init(KeyStaticParam_s *config, KEY_Type_e KeyType)
{
    if (KeyType >= KEY_NUM) return;
    
    // 1. 复制静态配置
    key[KeyType].StaticParam = *config;
    GPIO_Input(config->GPIO_Port, config->GPIO_Pin, GPIO_MODE_INPUT);
    
    // 3. 初始化运行参数
    key[KeyType].RunningParam.LastState = KEY_RELEASED;
    key[KeyType].RunningParam.LastTick = 0;
}

/**
 * @brief 按键设备总初始化
 */
void KeyDeviceInit(void)
{
    KeyStaticParam_s config;

    // USER KEY 配置
    config.GPIO_Port = KEY_GPIO_Port;
    config.GPIO_Pin = KEY_Pin;
    config.PressLevel = KEY_LOW_LEVEL_PRESS; // 假设低电平按下
    config.Mode = KEY_MODE_NORMAL;
    Key_Init(&config, KEY_USER);
}

/**
 * @brief 获取按键触发事件
 * @return 1: 有效按下（已消抖）, 0: 无事件
 */
uint8_t Key_GetNum(KEY_Type_e KeyType)
{
    if (KeyType >= KEY_NUM) return 0;
    
    KEYInstance *instance = &key[KeyType];
    KEY_State_e currentState = GetKeyState(KeyType);
    uint8_t key_event = 0;
    if (currentState == KEY_RELEASED && instance->RunningParam.LastState == KEY_PRESSED)
    {
        if (GetKeyState(KeyType) == KEY_RELEASED)
        {
            key_event = 1;
        }
    }
    instance->RunningParam.LastState = currentState;
    return key_event;
}