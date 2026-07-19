#include "stm32f10x.h"
#include "ch438q.h"
#include "modbus.h"
#include "SP3485.h"
#include "74HC595.h"  

volatile uint32_t g_Tick = 0;

int main(void)
{
    uint8_t i;
    CH438_HardwareInit();  
    HC595_Init();  
    for(i = 0; i < 16; i++)
    {
        CH438_UART_Init(i);
    } 
    SP3485_Init();
    Modbus_Init();   
    CH438_SendString(10, "16-CH Modbus RTU Slave Started!\r\n");
    CH438_Printf(10, "UART0 Addr: 0x%02X\r\n", g_Modbus[0].slave_addr);
    CH438_Printf(10, "UART3 (485) Addr: 0x%02X\r\n", g_Modbus[3].slave_addr);    
    while(1)
    {
        if(g_Ch438IntFlag0)
        {
            g_Ch438IntFlag0 = 0;
            CH438_PollingAll(0);
        }
        if(g_Ch438IntFlag1)
        {
            g_Ch438IntFlag1 = 0;
            CH438_PollingAll(1);
        }
        
        static uint32_t last_poll = 0;
        if(g_Tick - last_poll > 10)
        {
            last_poll = g_Tick;
            CH438_PollingAll(0);
            CH438_PollingAll(1);
        }
        
        Modbus_Loop();			   
    }
}
