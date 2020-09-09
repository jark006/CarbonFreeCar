#ifndef __DISTANCE_H
#define __DISTANCE_H
#include <STC15F2K60S2.H>
#include <delay.h>
void Error_Correct_ad(u16 *aa, u8 n);
void InitADC();
u16 ADC(u8i way);
#endif