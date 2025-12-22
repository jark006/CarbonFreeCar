#ifndef __OLED_H_
#define __OLED_H_
#include "delay.h"

void Init_OLED();
void OLED_ShowChar(u8 x, u8 y, u8 chr, u8 Char_Size);
void OLED_ShowString(u8 x, u8 y, u8* chr, u8 Char_Size);
void OLED_ShowNum(u8 x, u8 y, u32 num, u8 len, u8 size2);
void OLED_Clear();

#endif