#include "headfile.h"
#include "ui.h"
#include "u8g2.h"
#include "stdio.h"
#include "math.h"

/**
 * @brief 绘制平衡模式页面（姿态监控）
 */
static void OLED_DrawBalancePage(float pitch) 
{
    char str[20];
    
    // 1. 顶部状态栏布局
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf); 
    u8g2_DrawStr(&u8g2, 0, 10, "BAL-MODE"); 

    // --- 右上方：运行时间 ---
    sprintf(str, "%lus", HAL_GetTick() / 1000);
    int time_w = u8g2_GetStrWidth(&u8g2, str);
    u8g2_DrawStr(&u8g2, 128 - time_w, 10, str); 
    u8g2_DrawHLine(&u8g2, 0, 12, 128); // 顶部分割线 (Y=12)

    // --- UI 整体下移逻辑：设置新的基准中心线 ---
    // 将中心从 38 下移到 40，这样圆顶部 (40-25=15) 距离分割线有 3 像素间隙
    int base_y = 40; 

    // 2. 动态水平线
    int horizon_y = base_y + (int)(pitch * 1.2f); 
    for(int i=0; i<128; i+=8) u8g2_DrawPixel(&u8g2, i, base_y); // 虚线参考线随之移动
    u8g2_DrawHLine(&u8g2, 25, horizon_y, 78); 

    // 3. 固定圆圈装饰
    u8g2_DrawCircle(&u8g2, 64, base_y, 25, U8G2_DRAW_ALL);

    // 4. Pitch 角度数字展示（基于 base_y 偏移）
    u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
    sprintf(str, "%.1f", pitch);
    int pitch_w = u8g2_GetStrWidth(&u8g2, str);
    // 原来是 43 (38+5)，现在改为 base_y + 5 = 45
    u8g2_DrawStr(&u8g2, 64 - (pitch_w / 2), base_y + 5, str);
    
    // 5. "PIT" 标签（基于 base_y 偏移）
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    int label_w = u8g2_GetStrWidth(&u8g2, "PIT");
    // 原来是 58 (38+20)，现在改为 base_y + 20 = 60
    u8g2_DrawStr(&u8g2, 64 - (label_w / 2), base_y + 20, "PIT");
}

/**
 * @brief 绘制蓝牙遥控模式页面
 * @param distance 当前超声波距离 (cm)
 */
static void OLED_DrawBluetoothPage(float distance, uint8_t is_connected) 
{
    char str[20];
    uint32_t tick = HAL_GetTick();
    
    // --- 1. 顶部状态栏 ---
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "BT-Mode"); 
    
    sprintf(str, " REAL:%d", (int)distance); 
    int dist_w = u8g2_GetStrWidth(&u8g2, str);
    u8g2_DrawStr(&u8g2, 128 - dist_w, 10, str); 
    u8g2_DrawHLine(&u8g2, 0, 12, 128); 

    // --- 3. 两边空白区域的“点点”修饰 ---
    // 在左右两侧绘制一些对称的装饰点，模拟电子信号感
    for(int i=0; i<3; i++) {
        int offset = (HAL_GetTick() / 200 + i) % 4; // 简单的点阵动画
        u8g2_DrawPixel(&u8g2, 15, 20 + i*8 + offset);     // 左侧点
        u8g2_DrawPixel(&u8g2, 115, 20 + i*8 + offset);   // 右侧点
    }

    // --- 3. 中间核心区 (居中) ---
    u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
    const char* status_txt = is_connected ? "OK" : "NO";
    int txt_w = u8g2_GetStrWidth(&u8g2, status_txt);
    int total_content_w = 32 + 8 + txt_w;
    int start_x = (128 - total_content_w) / 2;

    u8g2_DrawXBMP(&u8g2, start_x, 16, 32, 32, icon_bt[0]);
    u8g2_DrawStr(&u8g2, start_x + 32 + 8, 38, status_txt);

    if (is_connected) {
        // 连接成功装饰波纹
        u8g2_DrawCircle(&u8g2, start_x + 16, 32, 13, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
        u8g2_DrawCircle(&u8g2, start_x + 16, 32, 17, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_LOWER_RIGHT);
    } else {
        // 未连接图标打叉
        u8g2_DrawLine(&u8g2, start_x, 18, start_x + 32, 46);
        u8g2_DrawLine(&u8g2, start_x + 32, 18, start_x, 46);
    }

    // --- 4. 底部预警能量条 ---
    u8g2_DrawHLine(&u8g2, 0, 50, 128); // 底部上方分割线
    
    // 逻辑：远离拉满，靠近掉格。设定 80cm 为安全满格
    int bar_max_w = 110;
    int bar_fill_w = (int)((distance / 80.0f) * bar_max_w);
    bar_fill_w = (bar_fill_w > bar_max_w) ? bar_max_w : (bar_fill_w < 0 ? 0 : bar_fill_w);

    // 绘制外框 (居中)
    int bar_x = (128 - bar_max_w) / 2;
    u8g2_DrawFrame(&u8g2, bar_x - 2, 54, bar_max_w + 4, 10); 
    u8g2_DrawBox(&u8g2, bar_x, 56, bar_fill_w, 6);

    // 分段擦除感 (每格 10 像素)
    u8g2_SetDrawColor(&u8g2, 0);
    for(int i = 10; i < bar_max_w; i += 10) {
        u8g2_DrawVLine(&u8g2, bar_x + i, 56, 6);
    }
    u8g2_SetDrawColor(&u8g2, 1);
}
/**
 * @brief 绘制超声波跟随模式页面
 * @param distance 实际距离 d (cm)
 * @param target_dist 目标跟随距离 D (cm)
 */
static void OLED_DrawFollowPage(float distance, float target_dist) 
{
    static uint8_t scan_angle = 0;
    char str[20];
    
    // --- 1. 顶部状态栏 (统一风格) ---
    u8g2_SetFont(&u8g2, u8g2_font_6x10_tf);
    u8g2_DrawStr(&u8g2, 0, 10, "FOL-Mode"); // [左上]
    
    sprintf(str, "T:%d", (int)target_dist);
    int t_width = u8g2_GetStrWidth(&u8g2, str);
    u8g2_DrawStr(&u8g2, 128 - t_width, 10, str); // [右上]
    
    u8g2_DrawHLine(&u8g2, 0, 12, 128); // 顶部分割线

    // --- 2. 动态雷达扫描区域 ---
    // 为了腾出底部空间，调整圆心和半径
    const int cx = 64; 
    const int cy = 50; // 圆心上移，确保扫描线不碰到下分割线

    // 绘制雷达刻度半圆 (半径缩小，防止撞线)
    u8g2_DrawCircle(&u8g2, cx, cy, 37, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
    u8g2_DrawCircle(&u8g2, cx, cy, 19, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);

    // 绘制扫描线
    scan_angle = (scan_angle + 6) % 180; 
    float rad = (180 - scan_angle) * 3.14159f / 180.0f;
    int line_x = cx + (int)(35 * cos(rad));
    int line_y = cy - (int)(35 * sin(rad));
    u8g2_DrawLine(&u8g2, cx, cy, line_x, line_y);

    // --- 3. 目标物体动态位置 ---
    // 映射关系：距离越近越靠下 (cy)，距离越远越靠上
    int ball_y = cy - (int)(distance * 0.6f); 
    if (ball_y < 18) ball_y = 18; // 限制在顶部分割线下方
    if (ball_y > 52) ball_y = 52; // 限制在底部分割线上方
    
    u8g2_DrawDisc(&u8g2, cx, ball_y, 2, U8G2_DRAW_ALL); // 目标点
    u8g2_DrawFrame(&u8g2, cx - 5, ball_y - 5, 10, 10);  // 锁定框

    // --- 4. 底部数据反馈区 ---
    u8g2_DrawHLine(&u8g2, 0, 53, 128); // *** 新增：底部分割线 ***

    u8g2_SetFont(&u8g2, u8g2_font_4x6_tf); // 使用极小字体
    
    // 绘制 DIFF (左对齐)
    float diff = distance - target_dist;
    sprintf(str, "DIFF:%d", (int)diff);
    u8g2_DrawStr(&u8g2, 2, 63, str);
    
    // 绘制 REAL (右对齐)
    sprintf(str, "REAL:%d", (int)distance);
    int r_width = u8g2_GetStrWidth(&u8g2, str);
    u8g2_DrawStr(&u8g2, 128 - r_width - 2, 63, str);
}

void UI_Show(void)
{
    u8g2_FirstPage(&u8g2);
    do {
        // --- 正常模式显示 ---
        switch (balance.mode) 
        {
            case 0: OLED_DrawBalancePage(mpu.pitch); break;
            case 1: OLED_DrawBluetoothPage(distance, balance.is_connected); break;
            case 2: OLED_DrawFollowPage(distance, dist.target); break;
        }
    } while (u8g2_NextPage(&u8g2));
}
