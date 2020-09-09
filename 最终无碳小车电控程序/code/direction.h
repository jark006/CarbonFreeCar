#ifndef __DERECTION_H
#define __DERECTION_H
#include <STC15F2K60S2.H>
#include <delay.h>
/**  Òý½Å¶¨Òå **********************************/
sbit d1 = P3^4;
sbit d2 = P3^5;
sbit d3 = P3^6;
sbit d4 = P3^7;
void TimerInit();
void  dji(u8 angle);
void dji2(u8 angle);
#endif