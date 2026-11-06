/**
 * @file Mcal_Pwr.c
 * @brief MCAL Power & MCU Reset - Thực thi lệnh WFI và System Reset trực tiếp trên phần cứng Cortex-M4.
 */
 
#include "main.h"
#include "Mcal_Pwr.h"

void Mcal_Pwr_EnterSleepWFI(void)
{
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);   
}

void Mcal_Pwr_SystemReset(void)
{
    __disable_irq();
    NVIC_SystemReset();
}
