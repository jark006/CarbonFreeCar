#include "HMC5883.h"

#define SCL2 P54            //IIC时钟引脚定义
#define SDA2 P55            //IIC数据引脚定义

#define SlaveAddress 0x3C      //定义器件在IIC总线中的从地址

void HMC5883_Start() {
    SDA2 = 1;                    //拉高数据线
    SCL2 = 1;                    //拉高时钟线
    delay5us();                  //延时
    SDA2 = 0;                    //产生下降沿
    delay5us();                  //延时
    SCL2 = 0;                    //拉低时钟线
}

void HMC5883_Stop() {
    SDA2 = 0;                    //拉低数据线
    SCL2 = 1;                    //拉高时钟线
    delay5us();                  //延时
    SDA2 = 1;                    //产生上升沿
    delay5us();                  //延时
}

void HMC5883_SendACK(bit ack) {
    SDA2 = ack;                  //写应答信号
    SCL2 = 1;                    //拉高时钟线
    delay5us();                  //延时
    SCL2 = 0;                    //拉低时钟线
    delay5us();                  //延时
}

bit HMC5883_RecvACK() {
    SCL2 = 1;                    //拉高时钟线
    delay5us();                  //延时
    CY = SDA2;                   //读应答信号
    SCL2 = 0;                    //拉低时钟线
    delay5us();                  //延时
    return CY;
}

void HMC5883_SendByte(u8 dat) {
    u8 i;
    for (i = 0; i < 8; i++) {
        dat <<= 1;               //移出数据的最高位
        SDA2 = CY;               //送数据口
        SCL2 = 1;                //拉高时钟线
        delay5us();              //延时
        SCL2 = 0;                //拉低时钟线
        delay5us();              //延时
    }
    HMC5883_RecvACK();
}

u8 HMC5883_RecvByte() {
    u8 i;
    u8 dat = 0;
    SDA2 = 1;                    //使能内部上拉,准备读取数据,
    for (i = 0; i < 8; i++) {
        dat <<= 1;
        SCL2 = 1;                //拉高时钟线
        delay5us();              //延时
        dat |= SDA2;             //读数据
        SCL2 = 0;                //拉低时钟线
        delay5us();              //延时
    }
    return dat;
}

void Single_Write_HMC5883(u8 REG_Address, u8 REG_data) {
    HMC5883_Start();                  //起始信号
    HMC5883_SendByte(SlaveAddress);   //发送设备地址+写信号
    HMC5883_SendByte(REG_Address);    //内部寄存器地址，请参考中文pdf
    HMC5883_SendByte(REG_data);       //内部寄存器数据，请参考中文pdf
    HMC5883_Stop();                   //发送停止信号
}

//连续读出HMC5883内部角度数据，地址范围 0x3 ~ 0x5
HMC_DATA Read_HMC5883() {
    HMC_DATA hmcData;
    u8 i;

    HMC5883_Start();                          //起始信号
    HMC5883_SendByte(SlaveAddress);           //发送设备地址+写信号
    HMC5883_SendByte(0x03);                   //发送存储单元地址，从0x3开始

    HMC5883_Start();                          //起始信号
    HMC5883_SendByte(SlaveAddress + 1);       //发送设备地址+读信号
    for (i = 0; i < 6; i++) {
        ((u8*)&hmcData)[i] = HMC5883_RecvByte();   //连续读取6个地址数据，存储中BUF
        HMC5883_SendACK(i == 5 ? 1 : 0);      //最后一个数据需要回 NOACK
    }
    HMC5883_Stop();                           //停止信号
    return hmcData;
}

//初始化HMC5883
void Init_HMC5883() {
    Single_Write_HMC5883(0x02, 0x00);
}