#include <direction.h>
bit flag = 0;
void TimerInit()
{
	AUXR &= 0x3B;		//��ʱ��0/1/2ʱ��12Tģʽ
						//T0x12 T1x12 UART_M0x6 T2R T2_C/T T2x12 EXTRAM S1ST2 
	TMOD=0x11;		   //��T0/T1Ϊ��ʽ1
	TH0=0;
	TL0=0;
	TH1=0;
	TL1=0;
	T2L = 0x00;		//���ö�ʱ��ֵ  10ms
	T2H = 0xDC;		//���ö�ʱ��ֵ
}
//����ת��
void dji(u8 angle) //����Ƕ� 45���� 135��
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
//ɲ��
void dji2(u8 angle) //����Ƕ� 45���� 135��
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