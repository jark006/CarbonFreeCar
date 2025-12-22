#ifndef __DELAY_H
#define __DELAY_H

#include "STC15F2K60S2.H"

#define u8       unsigned char
#define u16      unsigned int
#define u32      unsigned long
#define uint8_t  unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned long
#define u8i      unsigned char idata
#define u16i     unsigned int  idata
#define u32i     unsigned long idata
#define u8x      unsigned char xdata
#define u16x     unsigned int  xdata
#define u32x     unsigned long xdata

#define TRUE  1
#define FALSE 0

void delay5us();
void delay1ms();
void delayms(u16 ms);
void delays(u16 s);

#endif 