#ifndef __MODBUS_H
#define __MODBUS_H

#include "stm32f10x.h"

#define MODBUS_UART_NUM    16
#define MODBUS_REG_NUM     4

typedef struct
{
    uint8_t  slave_addr;
    uint16_t reg[MODBUS_REG_NUM];
} Modbus_Chn_t;

extern Modbus_Chn_t g_Modbus[MODBUS_UART_NUM];

void Modbus_Init(void);
void Modbus_Loop(void);

#endif
