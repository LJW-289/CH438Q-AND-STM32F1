#include "74HC595.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

// 定义引脚
#define SH_CP_PIN GPIO_Pin_0  // PA0
#define DS_PIN    GPIO_Pin_1  // PA1
#define ST_CP_PIN GPIO_Pin_2  // PA2

// 内部变量，记录当前输出状态
static uint16_t pd_output = 0x0000;

// 延时函数
static void Delay(uint32_t count) {
    while (count--) {
        __NOP();
    }
}

// 初始化74HC595相关的GPIO
void HC595_Init(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // 配置PA0, PA1, PA2为推挽输出
    GPIO_InitStructure.GPIO_Pin = SH_CP_PIN | DS_PIN | ST_CP_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 初始状态：ST_CP低，SH_CP低
    GPIO_ResetBits(GPIOA, SH_CP_PIN | ST_CP_PIN);
    GPIO_ResetBits(GPIOA, DS_PIN);  // DS初始低

    // 初始化输出为0
    HC595_WriteWord(pd_output);
}

// 发送一个字节到74HC595
void HC595_WriteByte(uint8_t data) {
    HC595_WriteWord((uint16_t)data);
}

// 发送一个 16 位字到两个串联的74HC595
void HC595_WriteWord(uint16_t data) {
    uint8_t i;

    // ST_CP低，开始传输
    GPIO_ResetBits(GPIOA, ST_CP_PIN);

    for (i = 0; i < 16; i++) {
        // 发送最高位
        if (data & 0x8000) {
            GPIO_SetBits(GPIOA, DS_PIN);
        } else {
            GPIO_ResetBits(GPIOA, DS_PIN);
        }

        // 脉冲SH_CP
        GPIO_SetBits(GPIOA, SH_CP_PIN);
        Delay(10);  // 短延时
        GPIO_ResetBits(GPIOA, SH_CP_PIN);
        Delay(10);

        // 移位
        data <<= 1;
    }

    // ST_CP高，锁存数据
    GPIO_SetBits(GPIOA, ST_CP_PIN);
    Delay(10);
    GPIO_ResetBits(GPIOA, ST_CP_PIN);
}

// 设置指定位为高电平
void PD_SetBit(uint16_t pin) {
    pd_output |= pin;
    HC595_WriteWord(pd_output);
}

// 清除指定位为低电平
void PD_ResetBit(uint16_t pin) {
    pd_output &= ~pin;
    HC595_WriteWord(pd_output);
}

// 翻转指定位
void PD_ToggleBit(uint16_t pin) {
    pd_output ^= pin;
    HC595_WriteWord(pd_output);
}

// 读取当前输出状态
uint16_t PD_ReadOutput(void) {
    return pd_output;
}
