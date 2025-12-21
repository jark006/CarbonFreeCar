#include "distance.h"

#define ADC_POWER   0x80 //ADC电源控制位
#define ADC_FLAG    0x10 //ADC完成标志
#define ADC_START   0x08 //ADC起始控制位
#define ADC_SPEEDLL 0x00 //540个时钟
#define ADC_SPEEDL  0x20 //360个时钟
#define ADC_SPEEDH  0x40 //180个时钟
#define ADC_SPEEDHH 0x60 //90个时钟

bit ADCflag = 0;

void quickSortAscending(uint16_t* arr, uint8_t length) {
    if (length <= 1) {
        return;
    }
    
    {
        // 选择中间元素作为基准
        uint16_t pivot = arr[length / 2];
        uint8_t i = 0;
        uint8_t j = length - 1;

        while (i <= j) {
            // 从左向右找到第一个大于等于基准的元素
            while (arr[i] < pivot) {
                i++;
            }
            // 从右向左找到第一个小于等于基准的元素
            while (arr[j] > pivot) {
                j--;
            }
            // 交换这两个元素
            if (i <= j) {
                uint16_t temp = arr[i];
                arr[i] = arr[j];
                arr[j] = temp;
                i++;
                j--;
            }
        }

        // 递归排序左右子数组
        if (j > 0) {
            quickSortAscending(arr, j + 1);
        }
        if (i < length - 1) {
            quickSortAscending(arr + i, length - i);
        }
    }
}

void adc_isr() INTERRUPT(5) {
    ADC_CONTR &= !ADC_FLAG; //清除ADC中断标志
    ADCflag = 1;
}

// 初始化ADC
void Init_ADC() {
    P1ASF = 0x0e; //设置P1低6位都为AD口
    ADC_RES = 0; //清除结果寄存器
    ADC_RESL = 0;
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START;
}

// 读取ADC值
u16 getADCValue(u8 channel) {
    ADCflag = 0;
    ADC_CONTR = ADC_POWER | ADC_SPEEDLL | ADC_START | (channel & 0x07);// 确保通道号在0-7之间
    while (~ADCflag);
    return (ADC_RES << 2) + (ADC_RESL & 0x03);
}