#ifndef __HMC5883_H
#define __HMC5883_H
#include <STC15F2K60S2.H>
#include <delay.h>
#define	SlaveAddress   0x3C	  //定义器件在IIC总线中的从地址
//****************************************
// 定义端口
//****************************************
sbit    SCL2=P5^4;			//IIC时钟引脚定义
sbit    SDA2=P5^5;			//IIC数据引脚定义
//以下是模拟iic使用函数-------------
//void Delay5us();
//void Delay5ms();
void HMC5883_Start();
void HMC5883_Stop();
void HMC5883_SendACK(bit ack);
bit  HMC5883_RecvACK();
void HMC5883_Sendu8(u8 dat);
u8 HMC5883_Recvu8();
void HMC5883_ReadPage();
void HMC5883_WritePage();
void Init_HMC5883(void);            //初始化5883
void  Single_Write_HMC5883(uchar REG_Address,uchar REG_data);   //单个写入数据
uchar Single_Read_HMC5883(uchar REG_Address);                   //单个读取内部寄存器数据
void  Get_Direction();         //连续的读取内部寄存器数据
#endif