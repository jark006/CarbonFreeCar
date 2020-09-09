#include <direction.h>
bit flag = 0;
void TimerInit()
{
	AUXR &= 0x3B;		//定时器0/1/2时钟12T模式
						//T0x12 T1x12 UART_M0x6 T2R T2_C/T T2x12 EXTRAM S1ST2 
	TMOD=0x11;		   //设T0/T1为方式1
	TH0=0;
	TL0=0;
	TH1=0;
	TL1=0;
	T2L = 0x00;		//设置定时初值  10ms
	T2H = 0xDC;		//设置定时初值
}
//方向转角
void dji(u8 angle) //舵机角度 45°至 135°
{
  u8 i=2;
  u16 H,L;
  if(angle>135) angle = 135;
  if(angle<45 ) angle =  45;
  H = 65075 - angle*10.23;
  L = 47566 + angle*10.23;
  flag = 0;
  while(i--)
  {
    TH1 = H/256;
    TL1 = H%256;
    TR1 = 1;
    d1 = 1;
    while(~flag);
    flag = 0;
    TH1 = L/256;
    TL1 = L%256;
    TR1 = 1;
    d1 = 0;
    while(~flag);
    flag = 0;
  }
}
//刹车
void dji2(u8 angle) //舵机角度 45°至 135°
{
  u8 i=2;
  u16 H,L;
  if(angle>135) angle = 135;
  if(angle<45 ) angle =  45;
  H = 65075 - angle*10.23;
  L = 47566 + angle*10.23;
  flag = 0;
  while(i--)
  {
    TH1 = H/256;
    TL1 = H%256;
    TR1 = 1;
    d2 = 1;
    while(~flag);
    flag = 0;
    TH1 = L/256;
    TL1 = L%256;
    TR1 = 1;
    d2 = 0;
    while(~flag);
    flag = 0;
  }
}