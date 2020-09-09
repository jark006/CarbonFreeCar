#ifndef __DELAY_H
#define __DELAY_H	
#include <STC15F2K60S2.H>
#include <intrins.h>
#define uchar   unsigned char 
#define uint  unsigned int  
#define u8   unsigned char 
#define u16  unsigned int  
#define u32  unsigned long 
#define u8i  unsigned char idata
#define u16i unsigned int  idata
#define u32i unsigned long idata
#define u8x  unsigned char xdata
#define u16x unsigned int  xdata
#define u32x unsigned long xdata
sbit sw1 = P2^2;
sbit sw2 = P2^3;
sbit sw3 = P2^4;
sbit sw4 = P2^5;
void Delay5us();
void delay1ms()	;
void delayms(u16 ms);
void delays(u16 s);
#endif 