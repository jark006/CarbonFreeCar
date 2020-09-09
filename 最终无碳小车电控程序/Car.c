/*********************************************************************************
    2017ȫ����ѧ������ѵ���ۺ���������
			��̼С�������
			ָ����ʦ��������
		��Ա�����Խ� Ф��ȫ ��˼��
		
ע��������ʹ��Keil_uVision5��������ʹ��Keil_uVision5�򿪡�
Keil_uVision4�����°汾���ܻ���ֲ��������󣬿�ʹ��Keil_uVision4�����¹��̣�����ӱ�����C�ļ����ɡ�
**********************************************************************************/
#include <STC15F2K60S2.H>
#include <math.h>
#include <delay.h>
#include <serial.h>
#include <direction.h>
#include <distance.h>
#include <oled.h>
#include <HMC5883.h>
#include <MPU6050.h>
/**********************************************************************************
�궨��
*/
#define speed_add_up 50   //��������
#define speed_up     15   //�ٶ�����
#define speed_down   8    //�ٶ�����
#define BrakeOn_angle   93  //ɲ�ش��Ƕ�
#define BrakeOff_angle  70  //���ش��Ƕ�
#define angle_up     30  //���ƫ��
#define upslope_deviation  30  //����ʶ�����
#define slopetop_deviation 15  //�¶�ʶ�����
#define shortlimit     65
#define longlimit      80
#define priority_angle  8
#define count_up      100
#define safe_distance   30
#define emergency_angle  8
/***********************************************************************************
��������
*/
extern u8 BUF[8];
u16i time_count = 0;
u16i second_count = 0;
u8i speed_v = 6;
bit Brake_flag = 1;
bit On_upslope_flag = 0;
bit On_slopetop_flag = 0;
u8i  stop_angle = 90;
u8i ostop_angle = 90;
sbit cc = P3^3;
bit stop = 0;
bit Wait_flag = 0;
u16i  ai = 0,i,j;
u16i x,y,z,angle_direction;
u16i Vertical;
u8i first_direction,first_horizontal;
u8  Dic = 1;
u8i L[5] = {0};
u8i oL[5] = {0};
u8i left_dis,mid_dis,right_dis;
u8i state = 'A';
bit dd1 = 0;
bit dd2 = 0;
u8i angle = 90;
u8i oangle = 90;
float scaling = 0;  //���ҶԱȾ������
u8 deflection = 0;  //����ƫ��
u16i way_count_last = 0;
u16i way_count = 0;
u16i journey =0;
/**********************************************************************************
��������
*/
void Get_Original_Data();
void Get_ADC();
u8 Transform(u16 value);
void Angle_Cal();
void Drection_Cal();
void Show_Data();
void Brake_judge();
/**********************************************************************************
������
*/
void main()
{
	TimerInit();
	InitMPU6050();
	Init_HMC5883();
	Initlcd();
	InitADC(); //��ʼ��ADC
	IE = 0xae;
//ʹ�����жϿ��أ�ADC�ж�,�ⲿ�ж�1����ʱ��0��1�ж�
	//EA ELVD EADC ES ET1 EX1 ET0 EX0
	//1010 1110
	IE2 |= 0x04;
	OLED_Clear();
	OLED_ShowString(0,0,"Carbon-Free Car",16);
	for(ai=1; ai<104; ai++)
		OLED_ShowString(ai,2,"=>>",16);
	OLED_Clear();
	OLED_ShowString(8,0,"Left Mid Right",16);
	ai = 0;
	TR0 = 1;
	delayms(8);
	TR1 = 1;
	Get_Original_Data();  //��ȡ������
	while(1)
	{
		Get_Direction();   //�ų��������ݲɼ�
		Drection_Cal();    //���㷽���
		Brake_judge();     //ɲ���ж�
		Get_ADC();         //�������ݲɼ�
		Angle_Cal();       //����ת��
		Show_Data();       //��ʾ����
	}
}
/**********************************************************************************
��ȡ������
*/
void Get_Original_Data()
{
	u8i direcBUF[10];
	u8i all = 0;
	for(ai=0; ai<10; ai++)
	{
		Get_Direction();   //�ų��������ݲɼ�
		Drection_Cal();    //���㷽���
		direcBUF[ai] = Dic;
	}
	for(ai=0; ai<10; ai++)
		all += direcBUF[ai];
	first_direction = all/10;
	OLED_ShowNum(104,2,first_direction,1,16);
	first_horizontal = z;
}
/**********************************************************************************
���㷽���
*/
void Drection_Cal()
{
	x = BUF[0] << 8 | BUF[1];
	//z = BUF[2] << 8 | BUF[3];
	y = BUF[4] << 8 | BUF[5];
	if(x>300&&x<32768)
	{
		if(y<65350)
			Dic = 1;
		else Dic = 2;
	}
	else if(x>32768&&x<65390)
	{
		if(y>65350)
			Dic = 5;
		else Dic = 6;
	}
	if(y<32768)
	{
		if(x<32768)
			Dic = 3;
		else Dic = 4;
	}
	else if(y<65100)
	{
		if(x>32768)
			Dic = 7;
		else Dic = 8;
	}
	Vertical = GetData(ACCEL_XOUT_H);
}
/**********************************************************************************
ɲ���ж�
*/
void Brake_judge()
{
	if(!On_slopetop_flag)
	{
		if(first_direction <= 3)
			if(Dic>=(first_direction+3)&&(Dic<=first_direction+5))
				Brake_flag = 0;
		if(first_direction >= 6)
			if(Dic>=(first_direction-5)&&Dic<=(first_direction-3))
				Brake_flag = 0;
		if(first_direction==4)
			if(Dic==1||Dic==7||Dic==8)
				Brake_flag = 0;
		if(first_direction==5)
			if(Dic==1||Dic==2||Dic==8)
				Brake_flag = 0;
	}
	if(Brake_flag==0)
	{
		state = 'B';
		if((Vertical-first_horizontal)>upslope_deviation||(first_horizontal-Vertical)>upslope_deviation)
		{
			On_upslope_flag = 1;
			state = 'C';
		}
	}
	if(On_upslope_flag)
		if((Vertical-first_horizontal)<slopetop_deviation||(first_horizontal-Vertical)<slopetop_deviation)
		{
			On_slopetop_flag = 1;
			state = 'D';
		}
	if(On_slopetop_flag && On_upslope_flag)
	{
		Brake_flag = 1;
		state = 'E';
	}
}
/**********************************************************************************
�������ݲɼ�
*/
void Get_ADC()
{
	u8 i,j;
	u16 ad[10] = {0};
	for(j = 1; j<4; j++)
	{
		for(i = 0; i<10; i++)
			ad[i] = ADC(j);
		Error_Correct_ad(ad,10);
		L[j] = Transform(ad[5]);
	}
	for(j = 1; j<4; j++)
	{
		L[j]  = (L[j]+oL[j])/2;
		oL[j] = L[j];
	}
	left_dis  = L[1];
	mid_dis   = L[2];
	right_dis = L[3];
}
/**********************************************************************************
��������ת��
*/
u8 Transform(u16 advalue)
{
	if(advalue >500)
		advalue = 500;
	if(advalue < 75)
		advalue = 75;
	if(advalue > 125)
		return   22/(advalue*0.004883 - 0.18);
	else
		return 29.3/(advalue*0.004883 - 0.024);
}
/**********************************************************************************
��ʾ����
*/
void Show_Data()
{
	if(sw1==0)
	{
		OLED_ShowNum( 8,1,left_dis,3,16);
		OLED_ShowNum(43,1,mid_dis,3,16);
		OLED_ShowNum(86,1,right_dis,3,16);
		OLED_ShowNum(120,2,Dic,1,16);
	}
	if(sw2==0)
	{
		OLED_ShowNum(0, 3,journey,  5,16);
		OLED_ShowNum(43,3,way_count,4,16);
		OLED_ShowNum(43,2,way_count_last,4,16);
		OLED_ShowNum(86,3,angle,    3,16);
	}
	if(sw3==0)
	{
		OLED_ShowChar(80,2,state,16);
		OLED_ShowNum(0, 2,z,  5,16);
	}
}
/**********************************************************************************
����ƫת��
*/
void Angle_Cal()
{
	if(left_dis>right_dis)
	{
		scaling = (left_dis - right_dis) / right_dis;
		if(scaling>1) scaling = 1;
		deflection = scaling * angle_up;
		angle = 90-deflection;
		if(angle<45 ) angle =  45;
	}
	else
	{
		scaling = (right_dis - left_dis) / left_dis;
		if(scaling>1) scaling = 1;
		deflection = scaling * angle_up;
		angle = 90+deflection;
		if(angle>135) angle = 135;
	}
}
/***********************************************************************************
�жϴ�����
*/
void countITR() interrupt 2  //�ٶ��ж�ͳ��
{
	if(Wait_flag == 0)
	{
		T2L = 0x00;		//���ö�ʱ��ֵ  10ms 11.0592 12T
		T2H = 0xDC;		//���ö�ʱ��ֵ
		AUXR |=0X10;  //��ʼ��ʱ������ʱ
		EX1 = 0;
		Wait_flag = 1;
		way_count++;
		journey++;
	}
}
void clear_flag() interrupt 12  //��ʱ��������
{
	Wait_flag = 0; //�����־λ���رռ�ʱ
	AUXR &=0XEF;
	EX1 = 1;
}
void Direction_ctrl() interrupt 1  //������PWM����
{
	u8i  speed_v2;
	u16i H,L;
	TR0 = 0;
	TH0 = 0;
	TL0 = 0;
	if(dd1 == 0)
	{
		EX1 = 0;
		H = 65075 - oangle*10.23;
		TH0 = H/256;
		TL0 = H%256;
		d1  = 1;
		dd1 = 1;
		TR0 = 1;
	}
	else
	{
		EX1 = 1;
		L = 47566 + oangle*10.23;
		TH0 = L/256;
		TL0 = L%256;
		d1  = 0;
		dd1 = 0;
		TR0 = 1;
		speed_v2 = (32-speed_v)*1.5;
		if(angle > oangle)
			oangle += ((angle - oangle)/speed_v2+2);
		else if(angle < oangle)
			oangle -= ((oangle - angle)/speed_v2+2);
	}
}
void Brake_ctrl() interrupt 3  //ɲ�����PWM����
{
	u16i H,L;
	TR1 = 0;
	TH1 = 0;
	TL1 = 0;
	if(dd2 == 0)
	{
		EX1 = 0;
		H = 65075 - ostop_angle*10.23;
		TH1 = H/256;
		TL1 = H%256;
		d2  = 1;
		dd2 = 1;
		TR1 = 1;
	}
	else
	{
		EX1 = 1;
		L = 47566 + ostop_angle*10.23;
		TH1 = L/256;
		TL1 = L%256;
		d2  = 0;
		dd2 = 0;
		TR1 = 1;
		time_count ++;
		second_count = time_count/50;
		ai++;
		if(ai >= 20)
		{
			ai = 0;
			if((way_count > way_count_last)&&(way_count - way_count_last)>speed_add_up)
				way_count = way_count_last + speed_add_up;
			way_count_last = way_count;
			speed_v = way_count;
			if(speed_v > 25) speed_v = 25;
			if(speed_v < 6)  speed_v = 6;
			if(Brake_flag)
			{
				if(way_count >= speed_up)
					ostop_angle = BrakeOn_angle;
				else if(way_count <= speed_down)
					ostop_angle = BrakeOff_angle;
			}
			else ostop_angle = BrakeOff_angle;
			way_count = 0;
		}
	}
}