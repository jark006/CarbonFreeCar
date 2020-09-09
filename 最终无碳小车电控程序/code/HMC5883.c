#include <HMC5883.h>
u8 BUF[8];                         //接收数据缓存区
/**************************************
起始信号
**************************************/
void HMC5883_Start()
{
	SDA2 = 1;                    //拉高数据线
	SCL2 = 1;                    //拉高时钟线
	Delay5us();                 //延时
	SDA2 = 0;                    //产生下降沿
	Delay5us();                 //延时
	SCL2 = 0;                    //拉低时钟线
}
/**************************************
停止信号
**************************************/
void HMC5883_Stop()
{
	SDA2 = 0;                    //拉低数据线
	SCL2 = 1;                    //拉高时钟线
	Delay5us();                 //延时
	SDA2 = 1;                    //产生上升沿
	Delay5us();                 //延时
}
/**************************************
发送应答信号
入口参数:ack (0:ACK 1:NAK)
**************************************/
void HMC5883_SendACK(bit ack)
{
	SDA2 = ack;                  //写应答信号
	SCL2 = 1;                    //拉高时钟线
	Delay5us();                 //延时
	SCL2 = 0;                    //拉低时钟线
	Delay5us();                 //延时
}
/**************************************
接收应答信号
**************************************/
bit HMC5883_RecvACK()
{
	SCL2 = 1;                    //拉高时钟线
	Delay5us();                 //延时
	CY = SDA2;                   //读应答信号
	SCL2 = 0;                    //拉低时钟线
	Delay5us();                 //延时
	return CY;
}
/**************************************
向IIC总线发送一个字节数据
**************************************/
void HMC5883_SendByte(u8 dat)
{
	u8 i;
	for (i=0; i<8; i++)         //8位计数器
	{
		dat <<= 1;              //移出数据的最高位
		SDA2 = CY;               //送数据口
		SCL2 = 1;                //拉高时钟线
		Delay5us();             //延时
		SCL2 = 0;                //拉低时钟线
		Delay5us();             //延时
	}
	HMC5883_RecvACK();
}
/**************************************
从IIC总线接收一个字节数据
**************************************/
u8 HMC5883_RecvByte()
{
	u8 i;
	u8 dat = 0;
	SDA2 = 1;                    //使能内部上拉,准备读取数据,
	for (i=0; i<8; i++)         //8位计数器
	{
		dat <<= 1;
		SCL2 = 1;                //拉高时钟线
		Delay5us();             //延时
		dat |= SDA2;             //读数据
		SCL2 = 0;                //拉低时钟线
		Delay5us();             //延时
	}
	return dat;
}
//***************************************************
void Single_Write_HMC5883(uchar REG_Address,uchar REG_data)
{
	HMC5883_Start();                  //起始信号
	HMC5883_SendByte(SlaveAddress);   //发送设备地址+写信号
	HMC5883_SendByte(REG_Address);    //内部寄存器地址，请参考中文pdf
	HMC5883_SendByte(REG_data);       //内部寄存器数据，请参考中文pdf
	HMC5883_Stop();                   //发送停止信号
}
//********单字节读取内部寄存器*************************
uchar Single_Read_HMC5883(uchar REG_Address)
{
	uchar REG_data;
	HMC5883_Start();                          //起始信号
	HMC5883_SendByte(SlaveAddress);           //发送设备地址+写信号
	HMC5883_SendByte(REG_Address);                   //发送存储单元地址，从0开始
	HMC5883_Start();                          //起始信号
	HMC5883_SendByte(SlaveAddress+1);         //发送设备地址+读信号
	REG_data=HMC5883_RecvByte();              //读出寄存器数据
	HMC5883_SendACK(1);
	HMC5883_Stop();                           //停止信号
	return REG_data;
}
//******************************************************
//
//连续读出HMC5883内部角度数据，地址范围0x3~0x5
//
//******************************************************
void Get_Direction(void)
{
	uchar i;
	HMC5883_Start();                          //起始信号
	HMC5883_SendByte(SlaveAddress);           //发送设备地址+写信号
	HMC5883_SendByte(0x03);                   //发送存储单元地址，从0x3开始
	HMC5883_Start();                          //起始信号
	HMC5883_SendByte(SlaveAddress+1);         //发送设备地址+读信号
	for (i=0; i<6; i++)                      //连续读取6个地址数据，存储中BUF
	{
		BUF[i] = HMC5883_RecvByte();          //BUF[0]存储数据
		if (i == 5)
		{
			HMC5883_SendACK(1);                //最后一个数据需要回NOACK
		}
		else
		{
			HMC5883_SendACK(0);                //回应ACK
		}
	}
	HMC5883_Stop();                          //停止信号
	delayms(5);
}
//初始化HMC5883，根据需要请参考pdf进行修改****
void Init_HMC5883()
{
	Single_Write_HMC5883(0x02,0x00);  //
}