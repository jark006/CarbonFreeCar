#ifndef __SERIAL_H
#define __SERIAL_H
#include <STC15F2K60S2.H>
#include <delay.h>
sbit TXD = P3^1;
void Delay103us();
void sendbyte(u8 byte);
void send(u8 *p);
#endif