#include <MPU6050.h>
//**************************************
//I2C起始信号
//**************************************
void I2C_Start()
{
	SDA3 = 1;                    //拉高数据线
	SCL3 = 1;                    //拉高时钟线
	Delay5us();                 //延时
	SDA3 = 0;                    //产生下降沿
	Delay5us();                 //延时
	SCL3 = 0;                    //拉低时钟线
}
//**************************************
//I2C停止信号
//**************************************
void I2C_Stop()
{
	SDA3 = 0;                    //拉低数据线
	SCL3 = 1;                    //拉高时钟线
	Delay5us();                 //延时
	SDA3 = 1;                    //产生上升沿
	Delay5us();                 //延时
}
//**************************************
//I2C发送应答信号
//入口参数:ack (0:ACK 1:NAK)
//**************************************
void I2C_SendACK(bit ack)
{
	SDA3 = ack;                  //写应答信号
	SCL3 = 1;                    //拉高时钟线
	Delay5us();                 //延时
	SCL3 = 0;                    //拉低时钟线
	Delay5us();                 //延时
}
//**************************************
//I2C接收应答信号
//**************************************
bit I2C_RecvACK()
{
	SCL3 = 1;                    //拉高时钟线
	Delay5us();                 //延时
	CY = SDA3;                   //读应答信号
	SCL3 = 0;                    //拉低时钟线
	Delay5us();                 //延时
	return CY;
}
//**************************************
//向I2C总线发送一个字节数据
//**************************************
void I2C_SendByte(u8i dat)
{
	u8i i;
	for (i=0; i<8; i++)         //8位计数器
	{
		dat <<= 1;              //移出数据的最高位
		SDA3 = CY;               //送数据口
		SCL3 = 1;                //拉高时钟线
		Delay5us();             //延时
		SCL3 = 0;                //拉低时钟线
		Delay5us();             //延时
	}
	I2C_RecvACK();
}
//**************************************
//从I2C总线接收一个字节数据
//**************************************
u8 I2C_RecvByte()
{
	u8i i;
	u8i dat = 0;
	SDA3 = 1;                    //使能内部上拉,准备读取数据,
	for (i=0; i<8; i++)         //8位计数器
	{
		dat <<= 1;
		SCL3 = 1;                //拉高时钟线
		Delay5us();             //延时
		dat |= SDA3;             //读数据
		SCL3 = 0;                //拉低时钟线
		Delay5us();             //延时
	}
	return dat;
}
//**************************************
//向I2C设备写入一个字节数据
//**************************************
void Single_WriteI2C(u8i REG_Address,u8i REG_data)
{
	I2C_Start();                  //起始信号
	I2C_SendByte(SlaveAddress);   //发送设备地址+写信号
	I2C_SendByte(REG_Address);    //内部寄存器地址，
	I2C_SendByte(REG_data);       //内部寄存器数据，
	I2C_Stop();                   //发送停止信号
}
//**************************************
//从I2C设备读取一个字节数据
//**************************************
u8 Single_ReadI2C(u8i REG_Address)
{
	u8i REG_data;
	I2C_Start();                   //起始信号
	I2C_SendByte(SlaveAddress);    //发送设备地址+写信号
	I2C_SendByte(REG_Address);     //发送存储单元地址，从0开始
	I2C_Start();                   //起始信号
	I2C_SendByte(SlaveAddress+1);  //发送设备地址+读信号
	REG_data=I2C_RecvByte();       //读出寄存器数据
	I2C_SendACK(1);                //接收应答信号
	I2C_Stop();                    //停止信号
	return REG_data;
}
//**************************************
//初始化MPU6050
//**************************************
void InitMPU6050()
{
	Single_WriteI2C(PWR_MGMT_1, 0x00);	//解除休眠状态
	Single_WriteI2C(SMPLRT_DIV, 0x07);
	Single_WriteI2C(CONFIG, 0x06);
	Single_WriteI2C(GYRO_CONFIG, 0x18);
	Single_WriteI2C(ACCEL_CONFIG, 0x01);
}
//**************************************
//合成数据
//**************************************
int GetData(u8i REG_Address)
{
	u8i H,L;
	H=Single_ReadI2C(REG_Address);
	L=Single_ReadI2C(REG_Address+1);
	return (H<<8)+L;   //合成数据
}
/*
		GetData(ACCEL_XOUT_H);	//显示X轴加速度
		GetData(ACCEL_YOUT_H);	//显示Y轴加速度
		GetData(ACCEL_ZOUT_H);	//显示Z轴加速度
		GetData(GYRO_XOUT_H);  	//显示X轴角速度
		GetData(GYRO_YOUT_H); 	//显示Y轴角速度
		GetData(GYRO_ZOUT_H); 	//显示Z轴角速度
*/