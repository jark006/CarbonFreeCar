/*********************************************************************************
2017全国大学生工程训练综合能力竞赛
无碳小车电控组

指导老师：李湘勤
组员：连辉杰 肖佳全 陈思杰
程序作者：陈思杰
**********************************************************************************/

#include "delay.h"
#include "hmc5883.h"
#include "mpu6050.h"
#include "distance.h"
#include "oled.h"
#include "serial.h"
#include "timer.h"

#define SPEED_ADD_UP 50       // 增度上限
#define SPEED_BRAKE_ON 15     // 触发刹车速度
#define SPEED_BRAKE_OFF 8     // 取消刹车速度
#define ANGLE_BRAKE_ON 93     // 刹重锤角度
#define ANGLE_BRAKE_OFF 70    // 放重锤角度
#define ANGLE_TURN_MAX 30     // 最大转向角度
#define ANGLE_MAX 120         // 最大角度
#define ANGLE_MIN 60          // 最小角度
#define upslope_deviation 30  // 上坡识别误差
#define slopetop_deviation 15 // 坡顶识别误差
#define SPEED_MAX 25          // 最大速度
#define SPEED_MIN 6           // 最小速度

u8 currentSpeed = 6;           // 当前速度 6 ~ 25
bit isNeedBrake = TRUE;        // 需要刹车标志位
bit isUpsloping = 0;           // 上坡中标志位
bit isOnSlopetop = 0;          // 在坡顶标志位
u8 servoBrakeAngle = 70;       // 重锤舵机角度
u16 horizontalValue = 0;       // 当前水平加速度值
u16 horizontalValueBegin = 0;  // 初始水平加速度值
u8 directionValueBegin = 0;    // 初始车身方向值
u8 directionValue = 1;         // 当前车身方向值
u16 leftDistance = 10;         // 左距离 毫米 mm 0~900
u16 rightDistance = 10;        // 右距离 毫米 mm 0~900
u8 walkingStage = 'A';         // 赛道阶段
u8 directionAngleTarget = 90;  // 目标方向舵机角度 60 ~ 120
u8 directionAngleCurrent = 90; // 当前方向舵机角度 60 ~ 120
u16 way_count_last = 0;        // 上次码盘计数
u16 way_count = 0;             // 码盘计数
u16 journey = 0;               // 总里程

// 开关引脚 用于设置显示哪些数据
#define sw1 P22
#define sw2 P23
#define sw3 P24
#define sw4 P25

#define SERVO_DIREC_PIN P34 // 舵机方向控制引脚
#define SERVO_BRAKE_PIN P35 // 舵机刹车控制引脚

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

    GetInitialData(); // 获取起步数据

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

    delayms(500);

    // 取10次平均
    for (i = 0; i < 10; i++) {
        UpdateAttitude(); // 更新姿态
        directionValueSum += directionValue;
        horizontalValueSum += horizontalValue;
        delayms(100);
    }

    directionValueBegin = (directionValueSum + 5) / 10;   // 四舍五入，平均
    horizontalValueBegin = (horizontalValueSum + 5) / 10;
    OLED_ShowNum(104, 2, directionValueBegin, 1, 16);

    delayms(500);
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
    static u16 distanceVecLast[5] = {0};
    u16 distanceVec[5] = {0};
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
    if (leftDistance > rightDistance) {
        u16 turnAngle =
            ANGLE_TURN_MAX * (leftDistance - rightDistance) / rightDistance;
        if (turnAngle > ANGLE_TURN_MAX)
            turnAngle = ANGLE_TURN_MAX;
        directionAngleTarget = 90 - turnAngle;
    } else {
        u16 turnAngle =
            ANGLE_TURN_MAX * (rightDistance - leftDistance) / leftDistance;
        if (turnAngle > ANGLE_TURN_MAX)
            turnAngle = ANGLE_TURN_MAX;
        directionAngleTarget = 90 + turnAngle;
    }
}

// 外部中断1  码盘计数
void countITR() INTERRUPT(2) USING(3) {
    EX1 = 0; // 关闭外部中断1
    way_count++;
    journey++;

    // 每次计数后启动定时器2进行消抖
    T2L = 0x00;   // 设置定时初值  10ms 11.0592 12T
    T2H = 0xDC;   // 设置定时初值
    AUXR |= 0X10; // 开始计时
}

// 定时2中断
void clear_flag() INTERRUPT(12) USING(3) {
    AUXR &= 0XEF; // 关闭定时器2
    EX1 = 1;      // 消抖完成
}

code u16 ANGLE2TIME_HIGH_TAB[] = {
    64461, 64450, 64440, 64430, 64420, 64410, 64399, 64389, 64379, 64369, 64358,
    64348, 64338, 64328, 64317, 64307, 64297, 64287, 64277, 64266, 64256, 64246,
    64236, 64225, 64215, 64205, 64195, 64184, 64174, 64164, 64154, 64144, 64133,
    64123, 64113, 64103, 64092, 64082, 64072, 64062, 64052, 64041, 64031, 64021,
    64011, 64000, 63990, 63980, 63970, 63959, 63949, 63939, 63929, 63919, 63908,
    63898, 63888, 63878, 63867, 63857, 63847,
};

code u16 ANGLE2TIME_LOW_TAB[] = {
    48179, 48190, 48200, 48210, 48220, 48230, 48241, 48251, 48261, 48271, 48282,
    48292, 48302, 48312, 48323, 48333, 48343, 48353, 48363, 48374, 48384, 48394,
    48404, 48415, 48425, 48435, 48445, 48456, 48466, 48476, 48486, 48496, 48507,
    48517, 48527, 48537, 48548, 48558, 48568, 48578, 48589, 48599, 48609, 48619,
    48629, 48640, 48650, 48660, 48670, 48681, 48691, 48701, 48711, 48721, 48732,
    48742, 48752, 48762, 48773, 48783, 48793,
};

// 方向舵机PWM控制 定时器0中断
void Direction_ctrl() INTERRUPT(1) USING(1) {
    static bit PWM_LEVEL = 0;
    u16 speedInvert;
    u16 timerValue;

    if (!PWM_LEVEL) {
        PWM_LEVEL = 1;
        SERVO_DIREC_PIN = 1;
        // timerValue = 65075 - directionAngleCurrent * 10.23;
        timerValue = ANGLE2TIME_HIGH_TAB[directionAngleCurrent - ANGLE_MIN];
        TH0 = timerValue >> 8;
        TL0 = timerValue & 0xFF;
    } else {
        PWM_LEVEL = 0;
        SERVO_DIREC_PIN = 0;
        // timerValue = 47566 + directionAngleCurrent * 10.23;
        timerValue = ANGLE2TIME_LOW_TAB[directionAngleCurrent - ANGLE_MIN];
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

        if (directionAngleCurrent > ANGLE_MAX) {
            directionAngleCurrent = ANGLE_MAX;
        } else if (directionAngleCurrent < ANGLE_MIN) {
            directionAngleCurrent = ANGLE_MIN;
        }
    }
}

// 刹车舵机PWM控制  定时器1中断
void Brake_ctrl() INTERRUPT(3) USING(2) {
    static u8 cnt20ms = 0;
    static bit PWM_LEVEL = 0;
    u16 timerValue;

    if (!PWM_LEVEL) {
        PWM_LEVEL = 1;
        SERVO_BRAKE_PIN = 1;
        // timerValue = 65075 - servoBrakeAngle * 10.23;
        timerValue = ANGLE2TIME_HIGH_TAB[directionAngleCurrent - ANGLE_MIN];
        TH1 = timerValue >> 8;
        TL1 = timerValue & 0xff;
    } else {
        PWM_LEVEL = 0;
        SERVO_BRAKE_PIN = 0;
        // timerValue = 47566 + servoBrakeAngle * 10.23;
        timerValue = ANGLE2TIME_LOW_TAB[directionAngleCurrent - ANGLE_MIN];
        TH1 = timerValue >> 8;
        TL1 = timerValue & 0xff;

        if (++cnt20ms >= 25) { // 每 500ms 统计一次速度
            cnt20ms = 0;
            if (way_count > (way_count_last + SPEED_ADD_UP))
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
