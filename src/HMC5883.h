#ifndef __HMC5883_H
#define __HMC5883_H
#include "delay.h"

typedef struct HMC_DATA {
    u16 x;
    u16 y;
    u16 z;
} HMC_DATA;

void Init_HMC5883(void); // 初始化 HMC5883
HMC_DATA Read_HMC5883(); // 读取磁场方向
#endif