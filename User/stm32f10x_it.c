#include "stm32f10x_it.h"
#include "ch438q.h"

extern volatile uint32_t g_Tick;

void NMI_Handler(void){}
void HardFault_Handler(void){ while(1); }
void MemManage_Handler(void){ while(1); }
void BusFault_Handler(void){ while(1); }
void UsageFault_Handler(void){ while(1); }
void SVC_Handler(void){}
void DebugMon_Handler(void){}
void PendSV_Handler(void){}

void SysTick_Handler(void)
{
    g_Tick++;
    CH438_NotifySysTick();
}

void EXTI9_5_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line6);
        g_Ch438IntFlag0 = 1;
    }

    if (EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line7);
        g_Ch438IntFlag1 = 1;
    }
}
