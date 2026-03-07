#include "oled.h"
#include "main.h"
#include "oled_font.h"
#include "bsp_delay.h"

// 定义OLED的I2C总线结构体
static iic_bus_t oled_bus = {
    .IIC_SDA_PORT = OLED_SDA_GPIO_Port,
    .IIC_SDA_PIN = OLED_SDA_Pin,
    .IIC_SCL_PORT = OLED_SCL_GPIO_Port,
    .IIC_SCL_PIN = OLED_SCL_Pin
};

u8g2_t u8g2 = {0}; // 定义u8g2结构体实例

/**
  * @brief  OLED写命令
  */
static void OLED_WriteCommand(uint8_t Command)
{
    IICStart(&oled_bus);
    IICSendByte(&oled_bus, OLED_ADDR);
    IICWaitAck(&oled_bus);
    IICSendByte(&oled_bus, 0x00); // 控制字节：写命令
    IICWaitAck(&oled_bus);
    IICSendByte(&oled_bus, Command);
    IICWaitAck(&oled_bus);
    IICStop(&oled_bus);
}

/**
  * @brief  OLED写数据
  */
static void OLED_WriteData(uint8_t Data)
{
    IICStart(&oled_bus);
    IICSendByte(&oled_bus, OLED_ADDR);
    IICWaitAck(&oled_bus);
    IICSendByte(&oled_bus, 0x40); // 控制字节：写数据
    IICWaitAck(&oled_bus);
    IICSendByte(&oled_bus, Data);
    IICWaitAck(&oled_bus);
    IICStop(&oled_bus);
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
static void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置高4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置低4位
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
		}
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
    delay_ms(5);
    IICInit(&oled_bus); // 使用通用驱动初始化引脚
    
    OLED_WriteCommand(0xAE); // 关闭显示
    OLED_WriteCommand(0xD5); // 设置显示时钟分频
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8); // 设置多路复用率
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3); // 设置显示偏移
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40); // 设置显示开始行
    OLED_WriteCommand(0xA1); // 左右方向
    OLED_WriteCommand(0xC8); // 上下方向
    OLED_WriteCommand(0xDA); // COM引脚配置
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81); // 对比度
    OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9); // 预充电周期
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB); // VCOMH级别
    OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4); // 全屏点亮/正常
    OLED_WriteCommand(0xA6); // 正常显示
    OLED_WriteCommand(0x8D); // 充电泵
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF); // 开启显示
        
    OLED_Clear();            // 清屏
}


static uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch(msg)
    {
        case U8X8_MSG_DELAY_MILLI:
            delay_ms(arg_int);
            break;
        case U8X8_MSG_DELAY_I2C:
            /* arg_int is 1 or 4: 100KHz (5us) or 400KHz (1.25us) */
            // delay_us(arg_int<=2?5:1);
            break;
        
        case U8X8_MSG_GPIO_I2C_CLOCK:
            if ( arg_int == 0 )
            {
                HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_RESET);
            }
            else
            {
                HAL_GPIO_WritePin(OLED_SCL_GPIO_Port, OLED_SCL_Pin, GPIO_PIN_SET);
            }
            break;
        case U8X8_MSG_GPIO_I2C_DATA:
            if ( arg_int == 0 )
            {
                HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
            }
            else
            {
                HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
            }
            break;
        default:
            u8x8_SetGPIOResult(u8x8, 1);
            break;
    }
    return 1;
}

void U8G2_Init(u8g2_t *u8g2)
{
    OLED_Init();
    // 2. 注册 U8G2
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay);
    
    // 3. 运行初始化序列
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0); 
    u8g2_ClearBuffer(u8g2);
    
    u8g2_SetDrawColor(u8g2, 1); 
    u8g2_SetFont(u8g2, u8g2_font_6x10_tf); // 设置一个初始字体
}

