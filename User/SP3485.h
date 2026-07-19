#ifndef __SP3485_H
#define __SP3485_H

#include "stm32f10x.h"
#include "74HC595.h"

#define SP3485_MAX_CHN  16

typedef struct {
    uint8_t  uart_num;
    uint16_t pd_pin;
} SP3485_Chn_t;

void SP3485_Init(void);
void SP3485_EnterTx(uint8_t uart_num);
void SP3485_EnterRx(uint8_t uart_num);
uint8_t SP3485_IsRS485(uint8_t uart_num);

#endif
