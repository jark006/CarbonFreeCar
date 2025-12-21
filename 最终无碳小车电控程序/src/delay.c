#include "delay.h"

// ÑÓÊ±º¯Êý
void delay5us() {
    u8i a = 20;
    while (a--);
}

//@11.0592MHz STC15 1T
void delay1ms() {
    u8 i, j;
    _nop_();
    _nop_();
    _nop_();
    i = 11;
    j = 190;
    do {
        while (--j);
    } while (--i);
}

void delayms(u16 ms) {
    while (ms--)
        delay1ms();
}

void delays(u16 s) {
    while (s--)
        delayms(1000);
}
