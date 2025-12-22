#include "serial.h"

//模拟串口发送函数

//@11.0592MHz 1T 9600
static void delay104us() {
    u8 i, j;
    _nop_();
    _nop_();
    i = 2;
    j = 26;
    do {
        while (--j);
    } while (--i);
}

void sendByte(u8 byte) {
    u8 i = 8;
    TXD = 0;
    delay104us();
    while (i--) {
        TXD = (byte & 0x01); //先传低位
        delay104us();
        byte = byte >> 1;
    }
    TXD = 1;
    delay104us();
}

void sendStr(u8* ptr) {
    while (*ptr)
        sendByte(*ptr++);
}