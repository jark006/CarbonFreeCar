#include "MPU6050.h"

#define SCL3 P30  //IIC时钟引脚定义
#define SDA3 P31  //IIC数据引脚定义

// MPU6050 内部地址
#define    SMPLRT_DIV      0x19    //陀螺仪采样率，典型值：0x07(125Hz)
#define    CONFIG          0x1A    //低通滤波频率，典型值：0x06(5Hz)
#define    GYRO_CONFIG     0x1B    //陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define    ACCEL_CONFIG    0x1C    //加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)
#define    ACCEL_XOUT_H    0x3B
#define    ACCEL_XOUT_L    0x3C
#define    ACCEL_YOUT_H    0x3D
#define    ACCEL_YOUT_L    0x3E
#define    ACCEL_ZOUT_H    0x3F
#define    ACCEL_ZOUT_L    0x40
#define    TEMP_OUT_H      0x41
#define    TEMP_OUT_L      0x42
#define    GYRO_XOUT_H     0x43
#define    GYRO_XOUT_L     0x44
#define    GYRO_YOUT_H     0x45
#define    GYRO_YOUT_L     0x46
#define    GYRO_ZOUT_H     0x47
#define    GYRO_ZOUT_L     0x48
#define    PWR_MGMT_1      0x6B    //电源管理，典型值：0x00(正常启用)
#define    WHO_AM_I        0x75    //IIC地址寄存器(默认数值0x68，只读)
#define    SlaveAddress    0xD0    //IIC写入时的地址字节数据，+1为读取

void I2C_Start() {
    SDA3 = 1;                    //拉高数据线
    SCL3 = 1;                    //拉高时钟线
    delay5us();                  //延时
    SDA3 = 0;                    //产生下降沿
    delay5us();                  //延时
    SCL3 = 0;                    //拉低时钟线
}

void I2C_Stop() {
    SDA3 = 0;                    //拉低数据线
    SCL3 = 1;                    //拉高时钟线
    delay5us();                  //延时
    SDA3 = 1;                    //产生上升沿
    delay5us();                  //延时
}

void I2C_SendACK(bit ack) {
    SDA3 = ack;                  //写应答信号
    SCL3 = 1;                    //拉高时钟线
    delay5us();                  //延时
    SCL3 = 0;                    //拉低时钟线
    delay5us();                  //延时
}

bit I2C_RecvACK() {
    bit ack;
    SCL3 = 1;                    //拉高时钟线
    delay5us();                  //延时
    ack = SDA3;                  //读应答信号
    SCL3 = 0;                    //拉低时钟线
    delay5us();                  //延时
    return ack;
}

void I2C_SendByte(u8 dat) {
    u8 i;
    for (i = 0; i < 8; i++){
        SDA3 = dat & 0x80;      //送数据口
        dat <<= 1;              //移出数据的最高位
        SCL3 = 1;               //拉高时钟线
        delay5us();             //延时
        SCL3 = 0;               //拉低时钟线
        delay5us();             //延时
    }
    I2C_RecvACK();
}

u8 I2C_RecvByte() {
    u8 i;
    u8 dat = 0;
    SDA3 = 1;                    //使能内部上拉,准备读取数据,
    for (i = 0; i < 8; i++)  {
        dat <<= 1;
        SCL3 = 1;                //拉高时钟线
        delay5us();              //延时
        dat |= SDA3;             //读数据
        SCL3 = 0;                //拉低时钟线
        delay5us();              //延时
    }
    return dat;
}

void Single_WriteI2C(u8 REG_Address, u8 REG_data) {
    I2C_Start();                  //起始信号
    I2C_SendByte(SlaveAddress);   //发送设备地址+写信号
    I2C_SendByte(REG_Address);    //内部寄存器地址
    I2C_SendByte(REG_data);       //内部寄存器数据
    I2C_Stop();                   //发送停止信号
}

u8 Single_ReadI2C(u8 REG_Address) {
    u8 REG_data;
    I2C_Start();                   //起始信号
    I2C_SendByte(SlaveAddress);    //发送设备地址+写信号
    I2C_SendByte(REG_Address);     //发送存储单元地址，从0开始
    I2C_Start();                   //起始信号
    I2C_SendByte(SlaveAddress + 1);//发送设备地址+读信号
    REG_data = I2C_RecvByte();     //读出寄存器数据
    I2C_SendACK(1);                //接收应答信号
    I2C_Stop();                    //停止信号
    return REG_data;
}

//初始化MPU6050
void Init_MPU6050() {
    Single_WriteI2C(PWR_MGMT_1, 0x00);    //解除休眠状态
    Single_WriteI2C(SMPLRT_DIV, 0x07);
    Single_WriteI2C(CONFIG, 0x06);
    Single_WriteI2C(GYRO_CONFIG, 0x18);
    Single_WriteI2C(ACCEL_CONFIG, 0x01);
}

//合成数据
u16 ReadMPU6050(u8 REG_Address) {
    u8 H, L;
    H = Single_ReadI2C(REG_Address);
    L = Single_ReadI2C(REG_Address + 1);
    return (H << 8) + L;   //合成数据
}

// X轴加速度
u16 ReadAccelX() {
    return ReadMPU6050(ACCEL_XOUT_H);
}

// // Y轴加速度
// u16 ReadAccelY() {
//     return ReadMPU6050(ACCEL_YOUT_H);
// }

// // Z轴加速度
// u16 ReadAccelZ() {
//     return ReadMPU6050(ACCEL_ZOUT_H);
// }

// // 获取X轴角速度
// u16 ReadGyroX() {
//     return ReadMPU6050(GYRO_XOUT_H);
// }

// // 获取Y轴角速度
// u16 ReadGyroY() {
//     return ReadMPU6050(GYRO_YOUT_H);
// }

// // 获取Z轴角速度
// u16 ReadGyroZ() {
//     return ReadMPU6050(GYRO_ZOUT_H);
// }
