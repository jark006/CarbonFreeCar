#include <serial.h>
/***************************************************************************************************************************************************
	ģ�⴮�ڷ��ͺ���*/
void Delay104us()		//@11.0592MHz 1T 9600
{
	u8 i, j;
	_nop_();
	_nop_();
	i = 2;
	j = 26;
	do
	{
		while (--j);
	}
	while (--i);
}
void sendbyte(u8 byte)
{
	u8 i=8;
	TXD = 0;
	Delay104us();
	while(i--)
	{
		TXD=(byte&0x01); //�ȴ���λ
		Delay104us();
		byte=byte>>1;
	}
	TXD = 1;
	Delay104us();
}
void send(u8 *p)
{
	while(*p)
		sendbyte(*p++);
}