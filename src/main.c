/*********************************************************************************
2017全国大学生工程训练综合能力竞赛
无碳小车电控组

指导老师：李湘勤
组员：连辉杰 肖佳全 陈思杰
程序作者：陈思杰
**********************************************************************************/

#include "HMC5883.h"
#include "MPU6050.h"
#include "delay.h"
#include "direction.h"
#include "distance.h"
#include "oled.h"
#include "serial.h"

#define SPEED_ADD_UP 50       // 增度上限
#define SPEED_BRAKE_ON 15     // 触发刹车速度
#define SPEED_BRAKE_OFF 8     // 取消刹车速度
#define ANGLE_BRAKE_ON 93     // 刹重锤角度
#define ANGLE_BRAKE_OFF 70    // 放重锤角度
#define ANGLE_TURN_MAX 30     // 最大转向角度
#define upslope_deviation 30  // 上坡识别误差
#define slopetop_deviation 15 // 坡顶识别误差
#define SPEED_MAX 25          // 最大速度
#define SPEED_MIN 6           // 最小速度

u8 currentSpeed = 6;           // 当前速度 6 ~ 25
bit isNeedBrake = TRUE;        // 需要刹车标志位
bit isUpsloping = 0;           // 上坡中标志位
bit isOnSlopetop = 0;          // 在坡顶标志位
u8 servoBrakeAngle = 90;       // 舵机刹车角度
bit isDebouncing = 0;          // 码盘计数消抖标志位
u16 horizontalValue = 0;       // 当前水平加速度值
u16 horizontalValueBegin = 0;  // 初始水平加速度值
u8 directionValueBegin = 0;    // 初始车身方向值
u8 directionValue = 1;         // 当前车身方向值
u8 leftDistance = 0;           // 左距离
u8 rightDistance = 0;          // 右距离
u8 walkingStage = 'A';         // 赛道阶段
u8 directionAngleTarget = 90;  // 目标方向舵机角度
u8 directionAngleCurrent = 90; // 当前方向舵机角度
u16 way_count_last = 0;        // 上次码盘计数
u16 way_count = 0;             // 码盘计数
u16 journey = 0;               // 总里程

// 开关引脚 用于设置显示哪些数据
sbit sw1 = P2 ^ 2;
sbit sw2 = P2 ^ 3;
sbit sw3 = P2 ^ 4;
sbit sw4 = P2 ^ 5;

sbit SERVO_DIRECTION_PIN = P3 ^ 4; // 舵机方向控制引脚
sbit SERVO_BRAKE_PIN = P3 ^ 5;     // 舵机刹车控制引脚

// 函数声明
void GetInitialData();
void MeasuringDistance();
void AngleCalculate();
void UpdateAttitude();
void DisplayData();
void StageJudge();

// 主函数
void main() {
    Init_MPU6050();
    Init_HMC5883();
    Init_OLED();
    Init_ADC();
    Init_Timer();

    // 使能总中断开关，ADC中断,外部中断1，定时器0、1中断
    // EA ELVD EADC ES ET1 EX1 ET0 EX0
    // 1010 1110
    IE = 0xae;
    IE2 |= 0x04;

    OLED_Clear();
    OLED_ShowString(8, 0, "Left     Right", 16);

    delayms(500);
    GetInitialData(); // 获取起步数据
    delayms(500);

    while (1) {
        UpdateAttitude();    // 更新姿态 朝向、倾斜度
        StageJudge();        // 赛道阶段判断
        MeasuringDistance(); // 距离数据采集
        AngleCalculate();    // 计算转角
        DisplayData();       // 显示数据
    }
}

// 获取起步数据
void GetInitialData() {
    u8 i;
    u16 directionValueSum = 0;
    u16 horizontalValueSum = 0;

    // 取10次平均
    for (i = 0; i < 10; i++) {
        UpdateAttitude(); // 更新姿态
        directionValueSum += directionValue;
        horizontalValueSum += horizontalValue;
        delayms(10);
    }

    directionValueBegin = (u16)(directionValueSum / 10.0 + 0.5);
    horizontalValueBegin = (u16)(horizontalValueSum / 10.0 + 0.5);
    OLED_ShowNum(104, 2, directionValueBegin, 1, 16);
}

// 更新姿态 朝向、倾斜度
void UpdateAttitude() {
    HMC_DATA hmcData;
    hmcData = Read_HMC5883();
    if (hmcData.x > 300 && hmcData.x < 32768) {
        if (hmcData.y < 65350)
            directionValue = 1;
        else
            directionValue = 2;
    } else if (hmcData.x > 32768 && hmcData.x < 65390) {
        if (hmcData.y > 65350)
            directionValue = 5;
        else
            directionValue = 6;
    }
    if (hmcData.y < 32768) {
        if (hmcData.x < 32768)
            directionValue = 3;
        else
            directionValue = 4;
    } else if (hmcData.y < 65100) {
        if (hmcData.x > 32768)
            directionValue = 7;
        else
            directionValue = 8;
    }

    horizontalValue = ReadAccelX(); // 倾斜度
}

// 赛道阶段判断
void StageJudge() {
    if (!isOnSlopetop) {
        if (directionValueBegin <= 3)
            if (directionValue >= (directionValueBegin + 3) &&
                (directionValue <= directionValueBegin + 5))
                isNeedBrake = 0;
        if (directionValueBegin >= 6)
            if (directionValue >= (directionValueBegin - 5) &&
                directionValue <= (directionValueBegin - 3))
                isNeedBrake = 0;
        if (directionValueBegin == 4)
            if (directionValue == 1 || directionValue == 7 ||
                directionValue == 8)
                isNeedBrake = 0;
        if (directionValueBegin == 5)
            if (directionValue == 1 || directionValue == 2 ||
                directionValue == 8)
                isNeedBrake = 0;
    }
    if (isNeedBrake == 0) {
        walkingStage = 'B';
        if ((horizontalValue - horizontalValueBegin) > upslope_deviation ||
            (horizontalValueBegin - horizontalValue) > upslope_deviation) {
            isUpsloping = 1;
            walkingStage = 'C';
        }
    }
    if (isUpsloping)
        if ((horizontalValue - horizontalValueBegin) < slopetop_deviation ||
            (horizontalValueBegin - horizontalValue) < slopetop_deviation) {
            isOnSlopetop = 1;
            walkingStage = 'D';
        }
    if (isOnSlopetop && isUpsloping) {
        isNeedBrake = 1;
        walkingStage = 'E';
    }
}

// 测量距离
void MeasuringDistance() {
    static u8 distanceVecLast[5] = {0};
    u8 distanceVec[5] = {0};
    u8 i, j;

    for (j = 1; j < 4; j++) {
        u16 ADValueVec[10];
        for (i = 0; i < 10; i++)
            ADValueVec[i] = getADCValue(j);
        quickSortAscending(ADValueVec, 10);
        distanceVec[j] = ADValue2Distance(ADValueVec[5]);
    }

    // 平滑处理
    for (j = 1; j < 4; j++) {
        distanceVec[j] = (distanceVec[j] + distanceVecLast[j]) / 2;
        distanceVecLast[j] = distanceVec[j];
    }

    leftDistance = distanceVec[1];
    rightDistance = distanceVec[3];
}

// 显示数据
void DisplayData() {
    if (sw1 == 0) {
        OLED_ShowNum(8, 1, leftDistance, 3, 16);
        OLED_ShowNum(86, 1, rightDistance, 3, 16);
        OLED_ShowNum(120, 2, directionValue, 1, 16);
    }
    if (sw2 == 0) {
        OLED_ShowNum(0, 3, journey, 5, 16);
        OLED_ShowNum(43, 3, way_count, 4, 16);
        OLED_ShowNum(43, 2, way_count_last, 4, 16);
        OLED_ShowNum(86, 3, directionAngleTarget, 3, 16);
    }
    if (sw3 == 0) {
        OLED_ShowChar(80, 2, walkingStage, 16);
    }
}

// 计算偏转角
void AngleCalculate() {
    float scaling; // 左右对比距离比例
    u8 deflection; // 左右偏角

    if (leftDistance > rightDistance) {
        scaling = (float)(leftDistance - rightDistance) / rightDistance;
        if (scaling > 1.0)
            scaling = 1.0;
        deflection = scaling * ANGLE_TURN_MAX;
        directionAngleTarget = 90 - deflection;
        if (directionAngleTarget < 45)
            directionAngleTarget = 45;
    } else {
        scaling = (float)(rightDistance - leftDistance) / leftDistance;
        if (scaling > 1.0)
            scaling = 1.0;
        deflection = scaling * ANGLE_TURN_MAX;
        directionAngleTarget = 90 + deflection;
        if (directionAngleTarget > 135)
            directionAngleTarget = 135;
    }
}

// 外部中断1  码盘计数
void countITR() INTERRUPT(2) {
    if (isDebouncing) // 抖动中
        return;

    way_count++;
    journey++;
    EX1 = 0; // 关闭外部中断1

    // 每次计数后启动定时器2进行消抖
    T2L = 0x00;   // 设置定时初值  10ms 11.0592 12T
    T2H = 0xDC;   // 设置定时初值
    AUXR |= 0X10; // 开始计时
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
    u16 speedInvert;
    u16 timerValue;

    if (!PWM_LEVEL) {
        PWM_LEVEL = 1;
        SERVO_DIRECTION_PIN = 1;
        timerValue = 65075 - directionAngleCurrent * 10.23;
        TH0 = timerValue >> 8;
        TL0 = timerValue & 0xFF;
    } else {
        PWM_LEVEL = 0;
        SERVO_DIRECTION_PIN = 0;
        timerValue = 47566 + directionAngleCurrent * 10.23;
        TH0 = timerValue >> 8;
        TL0 = timerValue & 0xFF;

        // 根据当前速度调整转向速度
        speedInvert = (32 - currentSpeed) * 3 / 2;
        if (directionAngleTarget > directionAngleCurrent)
            directionAngleCurrent +=
                ((directionAngleTarget - directionAngleCurrent) / speedInvert +
                 2);
        else if (directionAngleTarget < directionAngleCurrent)
            directionAngleCurrent -=
                ((directionAngleCurrent - directionAngleTarget) / speedInvert +
                 2);
    }
}

// 刹车舵机PWM控制  定时器1中断
void Brake_ctrl() INTERRUPT(3) {
    static u8 cnt20ms = 0;
    static bit PWM_LEVEL = 0;
    u16 timerValue;

    if (!PWM_LEVEL) {
        PWM_LEVEL = 1;
        SERVO_BRAKE_PIN = 1;
        timerValue = 65075 - servoBrakeAngle * 10.23;
        TH1 = timerValue >> 8;
        TL1 = timerValue & 0xff;
    } else {
        PWM_LEVEL = 0;
        SERVO_BRAKE_PIN = 0;
        timerValue = 47566 + servoBrakeAngle * 10.23;
        TH1 = timerValue >> 8;
        TL1 = timerValue & 0xff;

        if (++cnt20ms >= 25) { // 每 500ms 统计一次速度
            cnt20ms = 0;
            if ((way_count > way_count_last) &&
                (way_count - way_count_last) > SPEED_ADD_UP)
                way_count = way_count_last + SPEED_ADD_UP;
            way_count_last = way_count;
            currentSpeed = way_count;
            way_count = 0;

            if (currentSpeed > SPEED_MAX)
                currentSpeed = SPEED_MAX;
            if (currentSpeed < SPEED_MIN)
                currentSpeed = SPEED_MIN;
            if (isNeedBrake) {
                if (currentSpeed >= SPEED_BRAKE_ON)
                    servoBrakeAngle = ANGLE_BRAKE_ON;
                else if (currentSpeed <= SPEED_BRAKE_OFF)
                    servoBrakeAngle = ANGLE_BRAKE_OFF;
            } else
                servoBrakeAngle = ANGLE_BRAKE_OFF;
        }
    }
}
