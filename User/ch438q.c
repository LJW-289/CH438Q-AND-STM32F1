#include "ch438q.h"
#include "SP3485.h"
#include <stdarg.h>
#include <stdio.h>

#define CS0_PIN             GPIO_Pin_5
#define CS1_PIN             GPIO_Pin_7
#define WR_PIN              GPIO_Pin_0
#define RD_PIN              GPIO_Pin_1
#define ALE_PIN             GPIO_Pin_6

#define Fpclk 22118400

static const uint8_t addr_offset[8] = {0x00, 0x10, 0x20, 0x30, 0x08, 0x18, 0x28, 0x38};

static uint8_t  RxBuffer[16][CH438_MAX_BUF];
static uint8_t  RxCount[16];
static uint8_t  RxFrameFlag[16];
static uint32_t RxTimeoutCnt[16];

volatile uint8_t g_Ch438IntFlag0 = 0;
volatile uint8_t g_Ch438IntFlag1 = 0;

// ================= 底层并口操作 =================

static void SetInputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = 0xFF00;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void SetOutputMode(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = 0xFF00;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void CH438_Write(uint8_t chip, uint8_t addr, uint8_t data)
{
    uint16_t value;

    GPIO_SetBits(GPIOB, CS0_PIN);
    GPIO_SetBits(GPIOB, CS1_PIN);
    GPIO_SetBits(GPIOB, WR_PIN);
    GPIO_SetBits(GPIOB, RD_PIN);

    SetOutputMode();

    value = GPIO_ReadOutputData(GPIOB) & 0x00FF;
    GPIO_Write(GPIOB, ((uint16_t)addr << 8) | value);

    if (chip == 0)
        GPIO_ResetBits(GPIOB, CS0_PIN);
    else
        GPIO_ResetBits(GPIOB, CS1_PIN);

    GPIO_SetBits(GPIOB, ALE_PIN);
    GPIO_ResetBits(GPIOB, ALE_PIN);

    value = GPIO_ReadOutputData(GPIOB) & 0x00FF;
    GPIO_Write(GPIOB, ((uint16_t)data << 8) | value);

    GPIO_ResetBits(GPIOB, WR_PIN);
    GPIO_SetBits(GPIOB, WR_PIN);

    GPIO_SetBits(GPIOB, CS0_PIN);
    GPIO_SetBits(GPIOB, CS1_PIN);
}

uint8_t CH438_Read(uint8_t chip, uint8_t addr)
{
    uint8_t value;
    uint16_t port_val;

    GPIO_SetBits(GPIOB, CS0_PIN);
    GPIO_SetBits(GPIOB, CS1_PIN);
    GPIO_SetBits(GPIOB, WR_PIN);
    GPIO_SetBits(GPIOB, RD_PIN);

    SetOutputMode();

    port_val = GPIO_ReadOutputData(GPIOB) & 0x00FF;
    GPIO_Write(GPIOB, ((uint16_t)addr << 8) | port_val);

    if (chip == 0)
        GPIO_ResetBits(GPIOB, CS0_PIN);
    else
        GPIO_ResetBits(GPIOB, CS1_PIN);

    GPIO_SetBits(GPIOB, ALE_PIN);
    GPIO_ResetBits(GPIOB, ALE_PIN);

    SetInputMode();

    GPIO_ResetBits(GPIOB, RD_PIN);
    value = (uint8_t)(GPIO_ReadInputData(GPIOB) >> 8);
    GPIO_SetBits(GPIOB, RD_PIN);

    GPIO_SetBits(GPIOB, CS0_PIN);
    GPIO_SetBits(GPIOB, CS1_PIN);

    return value;
}

// ================= 初始化 =================

void CH438_HardwareInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = CS0_PIN | CS1_PIN | WR_PIN | RD_PIN | ALE_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIOB->BSRR = CS0_PIN | CS1_PIN | WR_PIN | RD_PIN;
    GPIOB->BSRR = ALE_PIN << 16;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);

    EXTI_InitStructure.EXTI_Line = EXTI_Line6 | EXTI_Line7;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    SysTick_Config(SystemCoreClock / 1000);
}

void CH438_UART_Init(uint8_t uart_num)
{
    uint8_t chip = (uart_num < 8) ? 0 : 1;
    uint8_t num  = uart_num % 8;
    uint8_t addr = addr_offset[num];
    uint16_t div = Fpclk / 192 / DEFAULT_BAUD_RATE;

    RxCount[uart_num] = 0;
    RxFrameFlag[uart_num] = 0;
    RxTimeoutCnt[uart_num] = 0;

    CH438_Write(chip, addr | REG_LCR_ADDR, 0x80);
    CH438_Write(chip, addr | REG_DLL_ADDR, (uint8_t)div);
    CH438_Write(chip, addr | REG_DLM_ADDR, (uint8_t)(div >> 8));
    CH438_Write(chip, addr | REG_LCR_ADDR, 0x03);
    CH438_Write(chip, addr | REG_FCR_ADDR, 0x07);
    CH438_Write(chip, addr | REG_IER_ADDR, 0x01);
    CH438_Write(chip, addr | REG_MCR_ADDR, 0x00);
}

// ================= 发送功能 =================

static void CH438_WaitTxComplete(uint8_t uart_num)
{
    uint8_t chip = (uart_num < 8) ? 0 : 1;
    uint8_t num  = uart_num % 8;
    uint8_t addr = addr_offset[num];
    uint8_t lsr;

    do {
        lsr = CH438_Read(chip, addr | REG_LSR_ADDR);
    } while (!(lsr & 0x40));
}

void CH438_SendByte(uint8_t uart_num, uint8_t Byte)
{
    uint8_t chip = (uart_num < 8) ? 0 : 1;
    uint8_t num  = uart_num % 8;
    uint8_t addr = addr_offset[num];
    while (!(CH438_Read(chip, addr | REG_LSR_ADDR) & 0x20));
    CH438_Write(chip, addr | REG_THR_ADDR, Byte);
}

void CH438_SendArray(uint8_t uart_num, uint8_t *Array, uint16_t Length)
{
    uint16_t i;

    if (SP3485_IsRS485(uart_num))
        SP3485_EnterTx(uart_num);

    for (i = 0; i < Length; i++)
        CH438_SendByte(uart_num, Array[i]);

    if (SP3485_IsRS485(uart_num))
    {
        CH438_WaitTxComplete(uart_num);
        SP3485_EnterRx(uart_num);
    }
}

void CH438_SendString(uint8_t uart_num, char *String)
{
    if (SP3485_IsRS485(uart_num))
        SP3485_EnterTx(uart_num);

    while (*String != '\0')
        CH438_SendByte(uart_num, *String++);

    if (SP3485_IsRS485(uart_num))
    {
        CH438_WaitTxComplete(uart_num);
        SP3485_EnterRx(uart_num);
    }
}

void CH438_SendNumber(uint8_t uart_num, uint32_t Number, uint8_t Length)
{
    static const uint32_t pow10[] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
    uint8_t i;
    for (i = 0; i < Length; i++)
        CH438_SendByte(uart_num, (Number / pow10[Length - i - 1]) % 10 + '0');
}

int fputc(int ch, FILE *f)
{
    CH438_SendByte(DEFAULT_UART, (uint8_t)ch);
    return ch;
}

void CH438_Printf(uint8_t uart_num, char *format, ...)
{
    char String[128];
    va_list arg;
    va_start(arg, format);
    vsnprintf(String, sizeof(String), format, arg);
    va_end(arg);
    CH438_SendString(uart_num, String);
}

// ================= 接收功能 =================

uint8_t CH438_GetRxFrame(uint8_t uart_num, uint8_t **outBuf, uint8_t *outLen)
{
    if (RxFrameFlag[uart_num] == 1)
    {
        *outBuf = RxBuffer[uart_num];
        *outLen = RxCount[uart_num];
        RxFrameFlag[uart_num] = 0;
        RxCount[uart_num] = 0;
        return 1;
    }
    return 0;
}

void CH438_PollingAll(uint8_t chip)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        uint8_t addr = addr_offset[i];
        uint8_t global_idx = (chip == 0) ? i : (8 + i);

        while (CH438_Read(chip, addr | REG_LSR_ADDR) & 0x01)
        {
            uint8_t data = CH438_Read(chip, addr | REG_RBR_ADDR);
            if (RxCount[global_idx] < CH438_MAX_BUF)
            {
                RxBuffer[global_idx][RxCount[global_idx]++] = data;
                RxTimeoutCnt[global_idx] = 0;
            }
        }
    }
}

void CH438_NotifySysTick(void)
{
    uint8_t i;
    for (i = 0; i < 16; i++)
    {
        if (RxCount[i] > 0 && RxFrameFlag[i] == 0)
        {
            RxTimeoutCnt[i]++;
            if (RxTimeoutCnt[i] >= FRAME_TIMEOUT_TICKS)
            {
                RxFrameFlag[i] = 1;
                RxTimeoutCnt[i] = 0;
            }
        }
    }
}
