#ifndef __HMC5883_H
#define __HMC5883_H
#include <STC15F2K60S2.H>
#include <delay.h>
#define	SlaveAddress   0x3C	  //����������IIC�����еĴӵ�ַ
//****************************************
// ����˿�
//****************************************
sbit    SCL2=P5^4;			//IICʱ�����Ŷ���
sbit    SDA2=P5^5;			//IIC�������Ŷ���
//������ģ��iicʹ�ú���-------------
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
void Init_HMC5883(void);            //��ʼ��5883
void  Single_Write_HMC5883(uchar REG_Address,uchar REG_data);   //����д������
uchar Single_Read_HMC5883(uchar REG_Address);                   //������ȡ�ڲ��Ĵ�������
void  Get_Direction();         //�����Ķ�ȡ�ڲ��Ĵ�������
#endif