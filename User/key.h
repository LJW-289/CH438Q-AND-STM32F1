#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

// 按键引脚定义
#define KEY0_PIN        GPIO_Pin_11
#define KEY1_PIN        GPIO_Pin_12
#define KEY_PORT        GPIOA
#define KEY_RCC         RCC_APB2Periph_GPIOA

// 按键返回值定义
#define KEY_NO_PRESS    0    // 无按键按下
#define KEY0_PRESS      1    // PA11 按下
#define KEY1_PRESS      2    // PA12 按下

// 函数声明
void KEY_Init(void);                // 按键初始化
uint8_t KEY_Scan(uint8_t mode);     // 按键扫描函数

#endif
