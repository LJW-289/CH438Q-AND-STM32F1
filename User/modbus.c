#include "modbus.h"
#include "ch438q.h"

Modbus_Chn_t g_Modbus[MODBUS_UART_NUM];

static const uint16_t g_RegDefaults[MODBUS_UART_NUM][MODBUS_REG_NUM] = {
    
{0x007B, 0xFFBE, 0x2211, 0x0022},  // CH1
		
    {0x0033, 0x0022, 0x0003, 0x0004},  // CH2
    {0x0003, 0x0002, 0x0003, 0x0004},  // CH3
    {0x0004, 0x0002, 0x0003, 0x0004},  // CH4
    {0x0005, 0x0002, 0x0003, 0x0004},  // CH5
    {0x0006, 0x0002, 0x0003, 0x0004},  // CH6
    {0x0007, 0x0002, 0x0003, 0x0004},  // CH7
    {0x0008, 0x0002, 0x0003, 0x0004},  // CH8
    {0x0009, 0x0002, 0x0003, 0x0004},  // CH9
    {0x000A, 0x0002, 0x0003, 0x0004},  // CH10
    {0x000B, 0x0002, 0x0003, 0x0004},  // CH11
    {0x000C, 0x0002, 0x0003, 0x0004},  // CH12
    {0x000D, 0x0002, 0x0003, 0x0004},  // CH13
    {0x000E, 0x0002, 0x0003, 0x0085},  // CH14
    {0x000F, 0x0002, 0x0003, 0x0004},  // CH15
    {0x0010, 0x0002, 0x0003, 0x0004},  // CH16
};

static uint16_t crc16(uint8_t *pData, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint16_t i, j;

    for (i = 0; i < len; i++)
    {
        crc ^= pData[i];
        for (j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

static void Modbus_03_Handler(uint8_t uart_num, uint8_t *rx_buf, uint8_t rx_len)
{
    uint8_t tx_buf[64];
    uint16_t start_addr, reg_cnt;
    uint16_t crc_res;
    uint16_t tx_idx = 0;
    uint16_t i;

    Modbus_Chn_t *chn = &g_Modbus[uart_num];
    start_addr = (rx_buf[2] << 8) | rx_buf[3];
    reg_cnt = (rx_buf[4] << 8) | rx_buf[5];

    tx_buf[tx_idx++] = chn->slave_addr;
    tx_buf[tx_idx++] = 0x03;
    tx_buf[tx_idx++] = reg_cnt * 2;

    for (i = 0; i < reg_cnt; i++)
    {
        uint16_t addr = start_addr + i;
        if (addr >= MODBUS_REG_NUM) break;

        tx_buf[tx_idx++] = (chn->reg[addr] >> 8) & 0xFF;
        tx_buf[tx_idx++] = chn->reg[addr] & 0xFF;
    }

    crc_res = crc16(tx_buf, tx_idx);
    tx_buf[tx_idx++] = crc_res & 0xFF;
    tx_buf[tx_idx++] = (crc_res >> 8) & 0xFF;

    CH438_SendArray(uart_num, tx_buf, tx_idx);
}

static void Modbus_06_Handler(uint8_t uart_num, uint8_t *rx_buf, uint8_t rx_len)
{
    uint8_t tx_buf[8];
    uint16_t reg_addr, reg_val;
    uint16_t crc_res;

    Modbus_Chn_t *chn = &g_Modbus[uart_num];
    reg_addr = (rx_buf[2] << 8) | rx_buf[3];
    reg_val = (rx_buf[4] << 8) | rx_buf[5];

    if (reg_addr < MODBUS_REG_NUM)
        chn->reg[reg_addr] = reg_val;

    tx_buf[0] = chn->slave_addr;
    tx_buf[1] = 0x06;
    tx_buf[2] = rx_buf[2];
    tx_buf[3] = rx_buf[3];
    tx_buf[4] = rx_buf[4];
    tx_buf[5] = rx_buf[5];

    crc_res = crc16(tx_buf, 6);
    tx_buf[6] = crc_res & 0xFF;
    tx_buf[7] = (crc_res >> 8) & 0xFF;

    CH438_SendArray(uart_num, tx_buf, 8);
}

void Modbus_Init(void)
{
    uint8_t i, j;
    for (i = 0; i < MODBUS_UART_NUM; i++)
    {
        g_Modbus[i].slave_addr = i + 1;
        for (j = 0; j < MODBUS_REG_NUM; j++)
            g_Modbus[i].reg[j] = g_RegDefaults[i][j];
    }
}

void Modbus_Loop(void)
{
    uint8_t *rx_buf;
    uint8_t rx_len;
    uint16_t crc_recv, crc_calc;
    uint8_t i;

    for (i = 0; i < MODBUS_UART_NUM; i++)
    {
        if (CH438_GetRxFrame(i, &rx_buf, &rx_len))
        {
            if (rx_len < 4) continue;
            if (rx_buf[0] != g_Modbus[i].slave_addr) continue;

            crc_recv = (rx_buf[rx_len-1] << 8) | rx_buf[rx_len-2];
            crc_calc = crc16(rx_buf, rx_len - 2);
            if (crc_recv != crc_calc) continue;

            switch (rx_buf[1])
            {
                case 0x03: Modbus_03_Handler(i, rx_buf, rx_len); break;
                case 0x06: Modbus_06_Handler(i, rx_buf, rx_len); break;
                default: break;
            }
        }
    }
}
