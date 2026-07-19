#include "SP3485.h"
#include "74HC595.h"

static const SP3485_Chn_t g_SP3485[SP3485_MAX_CHN] = {
    {0,  PD_0},  {1,  PD_1},  {2,  PD_2},  {3,  PD_3},
    {4,  PD_4},  {5,  PD_5},  {6,  PD_6},  {7,  PD_7},
    {8,  PD_8},  {9,  PD_9},  {10, PD_10}, {11, PD_11},
    {12, PD_12}, {13, PD_13}, {14, PD_14}, {15, PD_15},
};

static uint16_t g_UartToPin[16];

void SP3485_Init(void)
{
    uint8_t i;
    for (i = 0; i < 16; i++)
        g_UartToPin[i] = 0xFFFF;

    for (i = 0; i < SP3485_MAX_CHN; i++)
    {
        g_UartToPin[g_SP3485[i].uart_num] = g_SP3485[i].pd_pin;
        PD_ResetBit(g_SP3485[i].pd_pin);
    }
}

void SP3485_EnterTx(uint8_t uart_num)
{
    if (g_UartToPin[uart_num] != 0xFFFF)
        PD_SetBit(g_UartToPin[uart_num]);
}

void SP3485_EnterRx(uint8_t uart_num)
{
    if (g_UartToPin[uart_num] != 0xFFFF)
        PD_ResetBit(g_UartToPin[uart_num]);
}

uint8_t SP3485_IsRS485(uint8_t uart_num)
{
    return (g_UartToPin[uart_num] != 0xFFFF);
}
