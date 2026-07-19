#include "key.h"

/**
 * @brief  按键初始化
 * @note   PA11/PA12 配置为上拉输入，默认高电平，按下低电平
 */
void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 开启GPIOA时钟
    RCC_APB2PeriphClockCmd(KEY_RCC, ENABLE);

    // 配置 PA11 PA12 上拉输入
    GPIO_InitStruct.GPIO_Pin = KEY0_PIN | KEY1_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_Init(KEY_PORT, &GPIO_InitStruct);
}

/**
 * @brief  按键扫描函数
 * @param  mode: 0-不支持连按  1-支持连按
 * @retval 按键值: 0-无按下 1-KEY0 2-KEY1
 */
uint8_t KEY_Scan(uint8_t mode)
{
    static uint8_t key_up = 1;  // 按键松开标志

    // 检测按键是否松开
    if(mode) key_up = 1;        // 支持连按

    if(key_up && (GPIO_ReadInputDataBit(KEY_PORT, KEY0_PIN)==0 || GPIO_ReadInputDataBit(KEY_PORT, KEY1_PIN)==0))
    {
        // 软件消抖延时 10ms
        for(uint16_t i=0; i<1000; i++);
        key_up = 0;

        if(GPIO_ReadInputDataBit(KEY_PORT, KEY0_PIN) == 0) return KEY0_PRESS;
        if(GPIO_ReadInputDataBit(KEY_PORT, KEY1_PIN) == 0) return KEY1_PRESS;
    }
    else if(GPIO_ReadInputDataBit(KEY_PORT, KEY0_PIN)==1 && GPIO_ReadInputDataBit(KEY_PORT, KEY1_PIN)==1)
    {
        key_up = 1;
    }
    return KEY_NO_PRESS;  // 无按键按下
}
