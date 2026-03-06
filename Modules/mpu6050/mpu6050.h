#ifndef __MPU6050_H
#define __MPU6050_H

#include "gpio.h"
#include "bsp_iic.h"
#include <stdint.h>

#define	MPU_ADDR		0x68	// MPU6050 I2C地址
// MPU6050寄存器地址
#define MPU_SAMPLE_RATE_REG		0X19	// 采样频率分频器
#define MPU_CFG_REG				0X1A	// 配置寄存器
#define	GYRO_CONFIG		0x1B	// 陀螺仪自检及测量范围
#define	ACCEL_CONFIG	0x1C	// 加速计自检、测量范围及高通滤波频率
#define	ACCEL_XOUT_H	0x3B
#define	ACCEL_XOUT_L	0x3C
#define	ACCEL_YOUT_H	0x3D
#define	ACCEL_YOUT_L	0x3E
#define	ACCEL_ZOUT_H	0x3F
#define	ACCEL_ZOUT_L	0x40
#define	TEMP_OUT_H		0x41
#define	TEMP_OUT_L		0x42
#define	GYRO_XOUT_H		0x43
#define	GYRO_XOUT_L		0x44	
#define	GYRO_YOUT_H		0x45
#define	GYRO_YOUT_L		0x46
#define	GYRO_ZOUT_H		0x47
#define	GYRO_ZOUT_L		0x48
#define	PWR_MGMT_1		0x6B 
#define	PWR_MGMT_2		0x6C
#define	MPU_DEVICE_ID_REG 0x75	

#define MPU_FIFO_EN_REG			0X23	//FIFO使能寄存器
#define MPU_I2CMST_STA_REG		0X36	//IIC主机状态寄存器
#define MPU_INTBP_CFG_REG		0X37	//中断/旁路设置寄存器
#define MPU_INT_EN_REG			0X38	//中断使能寄存器
#define MPU_INT_STA_REG			0X3A	//中断状态寄存器
#define MPU_USER_CTRL_REG		0X6A	//用户控制寄存器

#define GYRO_250DPS            0
#define GYRO_500DPS            1
#define GYRO_1000DPS           2
#define GYRO_2000DPS           3

#define ACC_2G                 0
#define ACC_4G                 1 
#define ACC_8G                 2
#define ACC_16G                3    

typedef struct
{
    int16_t acc[3];  // 加速度计原始值
    int16_t gyro[3];  // 陀螺仪原始值
    float gyroBalance, accyAngle;
    float gyrozReal, gyroyReal;
    float roll, pitch;
}MPU_t;

void MPU_Init(void);
void MPU_Get_Angle(MPU_t *mpu);

extern MPU_t mpu;

#endif

