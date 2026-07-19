#ifndef __CH438Q_H
#define __CH438Q_H

#include "stm32f10x.h"
#include <stdio.h>

#define CH438_MAX_BUF      64
#define DEFAULT_BAUD_RATE  9600
#define DEFAULT_UART       0

#define FRAME_TIMEOUT_TICKS ((150000 / DEFAULT_BAUD_RATE) + 5)

#define REG_RBR_ADDR        0x00
#define REG_THR_ADDR        0x00
#define REG_IER_ADDR        0x01
#define REG_IIR_ADDR        0x02
#define REG_LCR_ADDR        0x03
#define REG_MCR_ADDR        0x04
#define REG_LSR_ADDR        0x05
#define REG_FCR_ADDR        0x02
#define REG_DLL_ADDR        0x00
#define REG_DLM_ADDR        0x01

void CH438_HardwareInit(void);
void CH438_UART_Init(uint8_t uart_num);

void CH438_SendByte(uint8_t uart_num, uint8_t Byte);
void CH438_SendArray(uint8_t uart_num, uint8_t *Array, uint16_t Length);
void CH438_SendString(uint8_t uart_num, char *String);
void CH438_SendNumber(uint8_t uart_num, uint32_t Number, uint8_t Length);
void CH438_Printf(uint8_t uart_num, char *format, ...);

uint8_t CH438_GetRxFrame(uint8_t uart_num, uint8_t **outBuf, uint8_t *outLen);
uint8_t CH438_Read(uint8_t chip, uint8_t addr);
void CH438_Write(uint8_t chip, uint8_t addr, uint8_t data);
void CH438_PollingAll(uint8_t chip);
void CH438_NotifySysTick(void);

extern volatile uint8_t g_Ch438IntFlag0;
extern volatile uint8_t g_Ch438IntFlag1;

#endif
