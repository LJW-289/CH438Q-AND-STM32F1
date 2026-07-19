#ifndef __74HC595_H
#define __74HC595_H

#include "stm32f10x.h"

#define PD_0  0x0001  // Q0
#define PD_1  0x0002  // Q1
#define PD_2  0x0004  // Q2
#define PD_3  0x0008  // Q3
#define PD_4  0x0010  // Q4
#define PD_5  0x0020  // Q5
#define PD_6  0x0040  // Q6
#define PD_7  0x0080  // Q7
#define PD_8  0x0100  // Q8
#define PD_9  0x0200  // Q9
#define PD_10 0x0400  // Q10
#define PD_11 0x0800  // Q11
#define PD_12 0x1000  // Q12
#define PD_13 0x2000  // Q13
#define PD_14 0x4000  // Q14
#define PD_15 0x8000  // Q15


void HC595_Init(void);
void HC595_WriteByte(uint8_t data);
void HC595_WriteWord(uint16_t data);
void PD_SetBit(uint16_t pin);
void PD_ResetBit(uint16_t pin);
void PD_ToggleBit(uint16_t pin);
uint16_t PD_ReadOutput(void);

#endif
