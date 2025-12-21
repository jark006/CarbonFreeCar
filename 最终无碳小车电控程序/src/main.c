/*********************************************************************************
2017全国大学生工程训练综合能力竞赛
无碳小车电控组

指导老师：李湘勤
组员：连辉杰 肖佳全 陈思杰
程序作者：陈思杰
**********************************************************************************/

#include <math.h>
#include "delay.h"
#include "serial.h"
#include "direction.h"
#include "distance.h"
#include "oled.h"
#include "HMC5883.h"
#include "MPU6050.h"


#define speed_add_up       50   //增度上限
#define speed_up           15   //速度上限
#define speed_down         8    //速度下限
#define BrakeOn_angle      93   //刹重锤角度
#define BrakeOff_angle     70   //放重锤角度
#define angle_up           30   //最大偏角
#define upslope_deviation  30   //上坡识别误差
#define slopetop_deviation 15   //坡顶识别误差


extern u8 HMC5883BUF[8];

u8 currentSpeed = 6;          // 当前速度
bit isNeedBrake = TRUE;       // 需要刹车标志位
bit isUpsloping = 0;          // 上坡中标志位
bit isOnSlopetop = 0;         // 在坡顶标志位
u8 servoBrakeAngle = 90;      // 舵机刹车角度
bit isDebouncing = 0;         // 码盘计数消抖标志位
u16 horizontalValue = 0;      // 当前水平加速度值
u16 horizontalValueBegin = 0; // 初始水平加速度值
u8 directionValueBegin = 0;   // 初始车身方向值
u8 directionValue = 1;        // 当前车身方向值
u8 leftDistance = 0;          // 左距离
u8 rightDistance = 0;         // 右距离
u8 state = 'A';               // 状态显示字符

u8 servoDirectionAngleTarget = 90;  // 目标方向舵机角度
u8 servoDirectionAngleCurrent = 90; // 当前方向舵机角度

u16i way_count_last = 0; // 上次码盘计数
u16i way_count = 0;      // 码盘计数
u16i journey = 0;        // 总里程

//  开关引脚
sbit sw1 = P2 ^ 2;
sbit sw2 = P2 ^ 3;
sbit sw3 = P2 ^ 4;
sbit sw4 = P2 ^ 5;

sbit SERVO_DIRECTION_PIN = P3 ^ 4; // 舵机方向控制引脚
sbit SERVO_BRAKE_PIN = P3 ^ 5;     // 舵机刹车控制引脚

// 函数声明
void Get_Original_Data();
void measuringDistance();
u8 adValue2Distance(u16 adValue); // ADC数值转距离 cm
void Angle_Cal();
void Drection_Cal();
void Show_Data();
void Brake_judge();

// 主函数
void main() {
    Init_Timer();
    Init_MPU6050();
    Init_HMC5883();
    Init_OLED();
    Init_ADC();

    IE = 0xae;
    //使能总中断开关，ADC中断,外部中断1，定时器0、1中断
    //EA ELVD EADC ES ET1 EX1 ET0 EX0
    //1010 1110

    IE2 |= 0x04;

    OLED_Clear();
    OLED_ShowString(0, 0, "Carbon-Free Car", 16);
    delayms(500);

    OLED_Clear();
    OLED_ShowString(8, 0, "Left     Right", 16);
    TR0 = 1;
    delayms(8);
    TR1 = 1;

    Get_Original_Data();  //获取起步数据

    while (1) {
        Read_HMC5883();      //磁场方向数据采集
        Drection_Cal();      //计算方向角
        Brake_judge();       //刹车判断
        measuringDistance(); //距离数据采集
        Angle_Cal();         //计算转角
        Show_Data();         //显示数据
    }
}

// 获取起步数据
void Get_Original_Data() {
    u8 i;
    u8 direcBUF[10];
    u8 all = 0;

    for (i = 0; i < 10; i++) {
        Read_HMC5883();    //磁场方向数据采集
        Drection_Cal();    //计算方向角
        direcBUF[i] = directionValue;
    }
    for (i = 0; i < 10; i++)
        all += direcBUF[i];
    directionValueBegin = all / 10;
    OLED_ShowNum(104, 2, directionValueBegin, 1, 16);
    horizontalValueBegin = ReadAccelX();
}

// 计算方向角
void Drection_Cal() {
    u16 x = HMC5883BUF[0] << 8 | HMC5883BUF[1];
    //u16 z = HMC5883BUF[2] << 8 | HMC5883BUF[3];
    u16 y = HMC5883BUF[4] << 8 | HMC5883BUF[5];
    if (x > 300 && x < 32768) {
        if (y < 65350)
            directionValue = 1;
        else directionValue = 2;
    }
    else if (x > 32768 && x < 65390) {
        if (y > 65350)
            directionValue = 5;
        else directionValue = 6;
    }
    if (y < 32768) {
        if (x < 32768)
            directionValue = 3;
        else directionValue = 4;
    }
    else if (y < 65100) {
        if (x > 32768)
            directionValue = 7;
        else directionValue = 8;
    }
    horizontalValue = ReadAccelX();
}

// 刹车判断
void Brake_judge() {
    if (!isOnSlopetop) {
        if (directionValueBegin <= 3)
            if (directionValue >= (directionValueBegin + 3) && (directionValue <= directionValueBegin + 5))
                isNeedBrake = 0;
        if (directionValueBegin >= 6)
            if (directionValue >= (directionValueBegin - 5) && directionValue <= (directionValueBegin - 3))
                isNeedBrake = 0;
        if (directionValueBegin == 4)
            if (directionValue == 1 || directionValue == 7 || directionValue == 8)
                isNeedBrake = 0;
        if (directionValueBegin == 5)
            if (directionValue == 1 || directionValue == 2 || directionValue == 8)
                isNeedBrake = 0;
    }
    if (isNeedBrake == 0) {
        state = 'B';
        if ((horizontalValue - horizontalValueBegin) > upslope_deviation || (horizontalValueBegin - horizontalValue) > upslope_deviation) {
            isUpsloping = 1;
            state = 'C';
        }
    }
    if (isUpsloping)
        if ((horizontalValue - horizontalValueBegin) < slopetop_deviation || (horizontalValueBegin - horizontalValue) < slopetop_deviation) {
            isOnSlopetop = 1;
            state = 'D';
        }
    if (isOnSlopetop && isUpsloping) {
        isNeedBrake = 1;
        state = 'E';
    }
}

// 测量距离
void measuringDistance() {
    static u8 distanceVecLast[5] = { 0 };
    u8i distanceVec[5] = { 0 };
    u8 i, j;

    for (j = 1; j < 4; j++) {
        u16 adValue[10];
        for (i = 0; i < 10; i++)
            adValue[i] = getADCValue(j);
        quickSortAscending(adValue, 10);
        distanceVec[j] = adValue2Distance(adValue[5]);
    }

    // 平滑处理
    for (j = 1; j < 4; j++) {
        distanceVec[j] = (distanceVec[j] + distanceVecLast[j]) / 2;
        distanceVecLast[j] = distanceVec[j];
    }

    leftDistance = distanceVec[1];
    rightDistance = distanceVec[3];
}

// ADC数值转距离 cm
u8 adValue2Distance(u16 adValue) {
    if (adValue > 500)
        adValue = 500;
    if (adValue < 75)
        adValue = 75;

    if (adValue > 125)
        return   22 / (adValue * 0.004883 - 0.18);
    else
        return 29.3 / (adValue * 0.004883 - 0.024);
}

// 显示数据
void Show_Data() {
    if (sw1 == 0) {
        OLED_ShowNum(8, 1, leftDistance, 3, 16);
        OLED_ShowNum(86, 1, rightDistance, 3, 16);
        OLED_ShowNum(120, 2, directionValue, 1, 16);
    }
    if (sw2 == 0) {
        OLED_ShowNum(0, 3, journey, 5, 16);
        OLED_ShowNum(43, 3, way_count, 4, 16);
        OLED_ShowNum(43, 2, way_count_last, 4, 16);
        OLED_ShowNum(86, 3, servoDirectionAngleTarget, 3, 16);
    }
    if (sw3 == 0) {
        OLED_ShowChar(80, 2, state, 16);
    }
}

// 计算偏转角
void Angle_Cal() {
    float scaling;  //左右对比距离比例
    u8 deflection;  //左右偏角

    if (leftDistance > rightDistance) {
        scaling = (float)(leftDistance - rightDistance) / rightDistance;
        if (scaling > 1.0) scaling = 1.0;
        deflection = scaling * angle_up;
        servoDirectionAngleTarget = 90 - deflection;
        if (servoDirectionAngleTarget < 45) servoDirectionAngleTarget = 45;
    }
    else {
        scaling = (float)(rightDistance - leftDistance) / leftDistance;
        if (scaling > 1.0) scaling = 1.0;
        deflection = scaling * angle_up;
        servoDirectionAngleTarget = 90 + deflection;
        if (servoDirectionAngleTarget > 135) servoDirectionAngleTarget = 135;
    }
}

// 外部中断1  码盘计数
void countITR() INTERRUPT(2) {
    if (isDebouncing) // 抖动中
        return;

    way_count++;
    journey++;
    EX1 = 0;           // 关闭外部中断1

    // 每次计数后启动定时器2进行消抖
    T2L = 0x00;        //设置定时初值  10ms 11.0592 12T
    T2H = 0xDC;        //设置定时初值
    AUXR |= 0X10;      //开始计时
    isDebouncing = 1;
}

// 定时2中断
void clear_flag() INTERRUPT(12) {
    isDebouncing = 0; // 消抖完成
    AUXR &= 0XEF;
    EX1 = 1;
}

// 方向舵机PWM控制 定时器0中断
void Direction_ctrl() INTERRUPT(1) {
    static bit PWM_LEVEL = 0;
    u8i  speed_v2;
    u16i H, L;
    TR0 = 0;
    TH0 = 0;
    TL0 = 0;
    if (!PWM_LEVEL) {
        PWM_LEVEL = 1;
        H = 65075 - servoDirectionAngleCurrent * 10.23;
        TH0 = H / 256;
        TL0 = H % 256;
        SERVO_DIRECTION_PIN = 1;
        TR0 = 1;
    }
    else {
        PWM_LEVEL = 0;
        L = 47566 + servoDirectionAngleCurrent * 10.23;
        TH0 = L / 256;
        TL0 = L % 256;
        SERVO_DIRECTION_PIN = 0;
        TR0 = 1;
        speed_v2 = (32 - currentSpeed) * 1.5;
        if (servoDirectionAngleTarget > servoDirectionAngleCurrent)
            servoDirectionAngleCurrent += ((servoDirectionAngleTarget - servoDirectionAngleCurrent) / speed_v2 + 2);
        else if (servoDirectionAngleTarget < servoDirectionAngleCurrent)
            servoDirectionAngleCurrent -= ((servoDirectionAngleCurrent - servoDirectionAngleTarget) / speed_v2 + 2);
    }
}

//刹车舵机PWM控制  定时器1中断
void Brake_ctrl() INTERRUPT(3) {
    static u8 cnt20ms = 0;
    static bit PWM_LEVEL = 0;
    u16i timerValue;

    TR1 = 0; //关闭定时器1

    if (!PWM_LEVEL) {
        PWM_LEVEL = 1;
        timerValue = 65075 - servoBrakeAngle * 10.23;
        TH1 = timerValue >> 8;
        TL1 = timerValue & 0xff;
        SERVO_BRAKE_PIN = 1;
        TR1 = 1;
    }
    else {
        PWM_LEVEL = 0;
        timerValue = 47566 + servoBrakeAngle * 10.23;
        TH1 = timerValue >> 8;
        TL1 = timerValue & 0xff;
        SERVO_BRAKE_PIN = 0;
        TR1 = 1;

        if (++cnt20ms >= 25) { // 每 500ms 统计一次速度
            cnt20ms = 0;
            if ((way_count > way_count_last) && (way_count - way_count_last) > speed_add_up)
                way_count = way_count_last + speed_add_up;
            way_count_last = way_count;
            currentSpeed = way_count;
            way_count = 0;

            if (currentSpeed > 25) currentSpeed = 25;
            if (currentSpeed < 6)  currentSpeed = 6;
            if (isNeedBrake) {
                if (way_count >= speed_up)
                    servoBrakeAngle = BrakeOn_angle;
                else if (way_count <= speed_down)
                    servoBrakeAngle = BrakeOff_angle;
            }
            else servoBrakeAngle = BrakeOff_angle;
        }
    }
}
