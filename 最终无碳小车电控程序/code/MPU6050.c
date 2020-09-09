#include <MPU6050.h>
//**************************************
//I2C��ʼ�ź�
//**************************************
void I2C_Start()
{
	SDA3 = 1;                    //����������
	SCL3 = 1;                    //����ʱ����
	Delay5us();                 //��ʱ
	SDA3 = 0;                    //�����½���
	Delay5us();                 //��ʱ
	SCL3 = 0;                    //����ʱ����
}
//**************************************
//I2Cֹͣ�ź�
//**************************************
void I2C_Stop()
{
	SDA3 = 0;                    //����������
	SCL3 = 1;                    //����ʱ����
	Delay5us();                 //��ʱ
	SDA3 = 1;                    //����������
	Delay5us();                 //��ʱ
}
//**************************************
//I2C����Ӧ���ź�
//��ڲ���:ack (0:ACK 1:NAK)
//**************************************
void I2C_SendACK(bit ack)
{
	SDA3 = ack;                  //дӦ���ź�
	SCL3 = 1;                    //����ʱ����
	Delay5us();                 //��ʱ
	SCL3 = 0;                    //����ʱ����
	Delay5us();                 //��ʱ
}
//**************************************
//I2C����Ӧ���ź�
//**************************************
bit I2C_RecvACK()
{
	SCL3 = 1;                    //����ʱ����
	Delay5us();                 //��ʱ
	CY = SDA3;                   //��Ӧ���ź�
	SCL3 = 0;                    //����ʱ����
	Delay5us();                 //��ʱ
	return CY;
}
//**************************************
//��I2C���߷���һ���ֽ�����
//**************************************
void I2C_SendByte(u8i dat)
{
	u8i i;
	for (i=0; i<8; i++)         //8λ������
	{
		dat <<= 1;              //�Ƴ����ݵ����λ
		SDA3 = CY;               //�����ݿ�
		SCL3 = 1;                //����ʱ����
		Delay5us();             //��ʱ
		SCL3 = 0;                //����ʱ����
		Delay5us();             //��ʱ
	}
	I2C_RecvACK();
}
//**************************************
//��I2C���߽���һ���ֽ�����
//**************************************
u8 I2C_RecvByte()
{
	u8i i;
	u8i dat = 0;
	SDA3 = 1;                    //ʹ���ڲ�����,׼����ȡ����,
	for (i=0; i<8; i++)         //8λ������
	{
		dat <<= 1;
		SCL3 = 1;                //����ʱ����
		Delay5us();             //��ʱ
		dat |= SDA3;             //������
		SCL3 = 0;                //����ʱ����
		Delay5us();             //��ʱ
	}
	return dat;
}
//**************************************
//��I2C�豸д��һ���ֽ�����
//**************************************
void Single_WriteI2C(u8i REG_Address,u8i REG_data)
{
	I2C_Start();                  //��ʼ�ź�
	I2C_SendByte(SlaveAddress);   //�����豸��ַ+д�ź�
	I2C_SendByte(REG_Address);    //�ڲ��Ĵ�����ַ��
	I2C_SendByte(REG_data);       //�ڲ��Ĵ������ݣ�
	I2C_Stop();                   //����ֹͣ�ź�
}
//**************************************
//��I2C�豸��ȡһ���ֽ�����
//**************************************
u8 Single_ReadI2C(u8i REG_Address)
{
	u8i REG_data;
	I2C_Start();                   //��ʼ�ź�
	I2C_SendByte(SlaveAddress);    //�����豸��ַ+д�ź�
	I2C_SendByte(REG_Address);     //���ʹ洢��Ԫ��ַ����0��ʼ
	I2C_Start();                   //��ʼ�ź�
	I2C_SendByte(SlaveAddress+1);  //�����豸��ַ+���ź�
	REG_data=I2C_RecvByte();       //�����Ĵ�������
	I2C_SendACK(1);                //����Ӧ���ź�
	I2C_Stop();                    //ֹͣ�ź�
	return REG_data;
}
//**************************************
//��ʼ��MPU6050
//**************************************
void InitMPU6050()
{
	Single_WriteI2C(PWR_MGMT_1, 0x00);	//�������״̬
	Single_WriteI2C(SMPLRT_DIV, 0x07);
	Single_WriteI2C(CONFIG, 0x06);
	Single_WriteI2C(GYRO_CONFIG, 0x18);
	Single_WriteI2C(ACCEL_CONFIG, 0x01);
}
//**************************************
//�ϳ�����
//**************************************
int GetData(u8i REG_Address)
{
	u8i H,L;
	H=Single_ReadI2C(REG_Address);
	L=Single_ReadI2C(REG_Address+1);
	return (H<<8)+L;   //�ϳ�����
}
/*
		GetData(ACCEL_XOUT_H);	//��ʾX����ٶ�
		GetData(ACCEL_YOUT_H);	//��ʾY����ٶ�
		GetData(ACCEL_ZOUT_H);	//��ʾZ����ٶ�
		GetData(GYRO_XOUT_H);  	//��ʾX����ٶ�
		GetData(GYRO_YOUT_H); 	//��ʾY����ٶ�
		GetData(GYRO_ZOUT_H); 	//��ʾZ����ٶ�
*/