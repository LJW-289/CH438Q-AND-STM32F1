# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

16-channel Modbus RTU slave device based on STM32F103C8 (Cortex-M3). Uses two CH438Q UART expansion chips (8 UARTs each) connected via a parallel bus, with SP3485 RS-485 transceivers whose DE/RE direction pins are controlled through cascaded 74HC595 shift registers.

## Build System

This is a **Keil µVision 5** project (no CLI build). The project file is `Project.uvprojx`.

- **Compiler:** ARMCC V5.06 update 6 (build 750)
- **Device:** STM32F103C8 (64KB Flash, 20KB RAM)
- **Preprocessor define:** `USE_STDPERIPH_DRIVER`
- **Include paths:** `.\Start`, `.\User`, `.\Library`
- **Output:** `.\Objects\Project` (AXF/HEX)
- **StdPeriph Library version:** V3.5.0

Open `Project.uvprojx` in Keil MDK-ARM to build, flash, and debug. No Makefile or command-line build is configured.

## Architecture

### Directory Layout

| Path | Purpose |
|------|---------|
| `Start/` | CMSIS core (core_cm3), device header (stm32f10x.h), system init, startup assembly (`startup_stm32f10x_md.s`) |
| `Library/` | STM32F10x StdPeriph Library — all peripheral drivers (GPIO, RCC, USART, SPI, TIM, etc.) |
| `User/` | Application code — all custom drivers and business logic |

### Component Architecture

The application is a layered Modbus RTU slave with 16 independent channels:

1. **`ch438q.c`** — CH438Q parallel-bus UART expansion driver. Two chips (chip 0 = UART 0-7, chip 1 = UART 8-15) share GPIOB data bus (PB8-PB15). Control signals: CS0 (PB5), CS1 (PB7), WR (PB0), RD (PB1), ALE (PB6). Each UART gets its own 64-byte RX buffer (`RxBuffer[16][64]`). Interrupts via PA6/PA7 (EXTI9_5, falling edge) set `g_Ch438IntFlag0`/`g_Ch438IntFlag1`.

2. **`74HC595.c`** — Two cascaded 74HC595 shift registers on PA0-PA2 (SH_CP, DS, ST_CP) provide 16 GPIO output bits. These bits control the DE/RE direction pins of the SP3485 transceivers.

3. **`SP3485.c`** — Manages RS-485 direction for all 16 channels via the 74HC595. The mapping table `g_SP3485[]` binds each UART index to a 74HC595 output pin. Auto-direction: `SP3485_EnterTx()` sets DE/RE high before TX, `SP3485_EnterRx()` returns to low after TX complete.

4. **`modbus.c`** — Modbus RTU protocol layer. Supports function codes 0x03 (Read Holding Registers) and 0x06 (Write Single Register). Each of the 16 channels has its own slave address (1-16) and 4 pre-configured holding registers. CRC16 is validated on every frame. `Modbus_Loop()` polls all 16 channels each iteration.

5. **`key.c`** — Two button inputs on PA11/PA12 with software debounce.

### Data Flow

```
CH438Q INT pin (PA6/PA7) → EXTI9_5_IRQHandler sets g_Ch438IntFlag
    → main loop: CH438_PollingAll(chip) reads FIFO data into RxBuffer[ch][]
    → SysTick_Handler: increments timeout counter → sets RxFrameFlag[ch] after 5ms idle
    → Modbus_Loop(): checks RxFrameFlag → validates address/CRC → dispatches to handler
    → Response sent via CH438_SendArray (which auto-toggles SP3485 direction for RS-485 channels)
```

### GPIO Allocation

| Pin | Function |
|-----|----------|
| PA0 | 74HC595 SH_CP (shift clock) |
| PA1 | 74HC595 DS (data) |
| PA2 | 74HC595 ST_CP (latch) |
| PA6 | CH438Q chip 0 INT |
| PA7 | CH438Q chip 1 INT |
| PA11-PA12 | Push buttons (pull-up) |
| PB0-PB1 | WR/RD |
| PB5 | CS0 (chip 0) |
| PB6 | ALE |
| PB7 | CS1 (chip 1) |
| PB8-PB15 | 8-bit parallel data bus |

### SysTick

Configured at 1ms tick. Used for:
- Frame timeout detection (5ms idle = end of Modbus frame)
- `g_Tick` free-running counter for main loop polling (10ms fallback poll)
