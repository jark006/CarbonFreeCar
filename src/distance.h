#ifndef __DISTANCE_H
#define __DISTANCE_H
#include "delay.h"

void quickSortAscending(u16* arr, u8 length);
void Init_ADC();
u16 getADCValue(u8 channel);
u8 ADValue2Distance(u16 ADValue);

#endif