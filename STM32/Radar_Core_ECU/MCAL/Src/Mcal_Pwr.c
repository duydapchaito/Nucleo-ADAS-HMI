/**
 * @file Mcal_Pwr.c
 * @brief MCAL Power Management - Quản lý chế độ ngủ tiết kiệm điện năng của vi điều khiển.
 */
 
#include "main.h"
#include "Mcal_Pwr.h"

/**
 * @brief Đưa vi điều khiển vào chế độ Sleep sử dụng lệnh WFI (Wait For Interrupt).
 * @note Toàn bộ ngoại vi (CAN, TIM, UART) vẫn hoạt động bình thường, CPU tự động tỉnh giấc khi có ngắt.
 */
void Mcal_Pwr_EnterSleepWFI(void)
{
    /* 
     * Đưa lõi Cortex-M4 vào chế độ Sleep bằng lệnh WFI.
     * Mọi ngoại vi (TIM1, TIM2, CAN1, DMA, SysTick) vẫn chạy bình thường.
     */
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);   
}
