#include <distance.h>
void Error_Correct_ad(u16 *aa, u8 n)
{
	u16i temp;
	u8 i,j;
	u16i a[10]= {0};
	for(i=0; i<n; i++) a[i] = aa[i];
	for(i=0; i<n-1; i++) //n个数,要进行n-1次比较,因为第一个数不用和自身比较.
		for(j=0; j<n-1-i; j++) //因为前面已经循环过i次,所以只用执行n-1-i次比较.
			if(a[j]<a[j+1])
			{
				temp=a[j];
				a[j]=a[j+1];
				a[j+1]=temp;
			}
	for(i=0; i<n; i++) aa[i] = a[i];
}
#define ADC_POWER 0x80 //ADC电源控制位
#define ADC_FLAG 0x10 //ADC完成标志
#define ADC_START 0x08 //ADC起始控制位
#define ADC_SPEEDLL 0x00 //540个时钟
#define ADC_SPEEDL 0x20 //360个时钟
#define ADC_SPEEDH 0x40 //180个时钟
#define ADC_SPEEDHH 0x60 //90个时钟
bit ADCflag = 0;
bit ADCing = 0;
/*
InitADC(); //初始化ADC
IE = 0xa0; //使能ADC中断
*/
void adc_isr() interrupt 5 using 1
{
	ADC_CONTR &= !ADC_FLAG; //清除ADC中断标志
	ADCflag = 1;
//	SendData(ch); //显示通道号
//	SendData(ADC_RES); //读取高8位结果并发送到串口
// SendData(ADC_RESL); //显示低2位结果
//	if (++ch > 1) ch = 0; //切换到下一个通道
//	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | ch;
}
/*----------------------------
初始化ADC
----------------------------*/
void InitADC()
{
//	P1ASF = 0xff; //设置P1口全部为AD口
	P1ASF = 0x0e; //设置P1低6位都为AD口
//	P1ASF = 0x03;
	ADC_RES = 0; //清除结果寄存器
	ADC_RESL= 0;
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | 0;
	//delays(2); //ADC上电并延时
}
u16 ADC(u8i way)
{
	u16i temp = 0;
	ADCing = 1;
	if(way > 8) way = 0;
	ADCflag = 0;
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | way;
	while(~ADCflag);
	temp = (ADC_RES<<2)+(ADC_RESL&0x03);
	ADCing = 0;
	return temp;
}