#include <distance.h>
void Error_Correct_ad(u16 *aa, u8 n)
{
	u16i temp;
	u8 i,j;
	u16i a[10]= {0};
	for(i=0; i<n; i++) a[i] = aa[i];
	for(i=0; i<n-1; i++) //n����,Ҫ����n-1�αȽ�,��Ϊ��һ�������ú�����Ƚ�.
		for(j=0; j<n-1-i; j++) //��Ϊǰ���Ѿ�ѭ����i��,����ֻ��ִ��n-1-i�αȽ�.
			if(a[j]<a[j+1])
			{
				temp=a[j];
				a[j]=a[j+1];
				a[j+1]=temp;
			}
	for(i=0; i<n; i++) aa[i] = a[i];
}
#define ADC_POWER 0x80 //ADC��Դ����λ
#define ADC_FLAG 0x10 //ADC��ɱ�־
#define ADC_START 0x08 //ADC��ʼ����λ
#define ADC_SPEEDLL 0x00 //540��ʱ��
#define ADC_SPEEDL 0x20 //360��ʱ��
#define ADC_SPEEDH 0x40 //180��ʱ��
#define ADC_SPEEDHH 0x60 //90��ʱ��
bit ADCflag = 0;
bit ADCing = 0;
/*
InitADC(); //��ʼ��ADC
IE = 0xa0; //ʹ��ADC�ж�
*/
void adc_isr() interrupt 5 using 1
{
	ADC_CONTR &= !ADC_FLAG; //���ADC�жϱ�־
	ADCflag = 1;
//	SendData(ch); //��ʾͨ����
//	SendData(ADC_RES); //��ȡ��8λ��������͵�����
// SendData(ADC_RESL); //��ʾ��2λ���
//	if (++ch > 1) ch = 0; //�л�����һ��ͨ��
//	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | ch;
}
/*----------------------------
��ʼ��ADC
----------------------------*/
void InitADC()
{
//	P1ASF = 0xff; //����P1��ȫ��ΪAD��
	P1ASF = 0x0e; //����P1��6λ��ΪAD��
//	P1ASF = 0x03;
	ADC_RES = 0; //�������Ĵ���
	ADC_RESL= 0;
	ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | 0;
	//delays(2); //ADC�ϵ粢��ʱ
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