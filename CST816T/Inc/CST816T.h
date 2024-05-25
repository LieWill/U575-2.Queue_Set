#ifndef __CST816T_H_
#define __CST816T_H_
#include "i2c.h"

typedef struct TouchPoint {
    uint8_t Gesture;
    uint8_t Finger;
    uint16_t X;
    uint16_t Y;
} TouchType;

void CST816_IIC_WriteREG(unsigned char reg, unsigned char date);
unsigned char CST816_IIC_ReadREG(unsigned char reg);
unsigned char CST816_IIC_RecvByte(void);
void CST816T_Init(void);
uint8_t CST816_GetAction(uint16_t *X, uint16_t *Y, uint8_t *Gesture);
// uint8_t CST816_GetAction(TouchType *Touch);
//  CST816寄存器
#define GestureID     0x01 // 手势寄存器
#define FingerNum     0x02 // 手指数量
#define XposH         0x03 // x高四位
#define XposL         0x04 // x低八位
#define YposH         0x05 // y高四位
#define YposL         0x06 // y低八位
#define ChipID        0xA7 // 芯片型号
#define MotionMask    0xEC // 触发动作
#define AutoSleepTime 0xF9 // 自动休眠
#define IrqCrl        0xFA // 中断控制
#define AutoReset     0xFB // 无手势休眠
#define LongPressTime 0xFC // 长按休眠
#define DisAutoSleep  0xFE // 使能低功耗模式

#endif