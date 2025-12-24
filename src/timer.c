#include "timer.h"

void Init_Timer() {
    // 定时器0/1/2时钟 12T 模式
    // T0x12 T1x12 UART_M0x6 T2R T2_C/T T2x12 EXTRAM S1ST2
    AUXR &= 0x3B;

    TMOD = 0x11; // 设T0/T1为方式1: 16 位不可重装载模式

    TH0 = 0;
    TL0 = 0;
    TH1 = 0;
    TL1 = 0;
    
    TR0 = 1;    // 启动定时器
    TR1 = 1;
}