#ifndef __HMC5883_H
#define __HMC5883_H
#include "delay.h"

void Init_HMC5883(void);       //初始化 HMC5883
void Read_HMC5883();           //读取磁场方向
#endif