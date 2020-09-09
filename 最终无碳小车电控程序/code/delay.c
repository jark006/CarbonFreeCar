#include <delay.h>
/***************************************************************************************************************************************************
		ÑÓÊ±º¯Êý*/
void Delay5us()
{
  u8i a=20;
  while(a--);
}
void delay1ms()		//@11.0592MHz STC15 1T
{
	u8 i, j;
	_nop_();
	_nop_();
	_nop_();
	i = 11;
	j = 190;
	do
	{
		while (--j);
	}
	while (--i);
}
void delayms(u16 ms)
{
	while(ms--)
		delay1ms();
}
void delays(u16 s)
{
	while(s--)
		delayms(1000);
}