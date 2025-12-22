#include "direction.h"

void Init_Timer() {
    AUXR &= 0x3B;        //定时器0/1/2时钟12T模式
    //T0x12 T1x12 UART_M0x6 T2R T2_C/T T2x12 EXTRAM S1ST2 
    TMOD = 0x11;           //设T0/T1为方式1
    TH0 = 0;
    TL0 = 0;
    TH1 = 0;
    TL1 = 0;
    T2L = 0x00;        //设置定时初值  10ms
    T2H = 0xDC;        //设置定时初值
    
    TR0 = 1;
    TR1 = 1;
}