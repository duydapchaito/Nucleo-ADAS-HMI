#include "main.h"
#include "Mcal_Pwr.h"

void Mcal_Pwr_EnterSleepWFI(void)
{
    /* 
    * - PWR_MAINREGULATOR_ON: Giữ nguyên áp chính để thức dậy nhanh nhất
    * - PWR_SLEEPENTRY_WFI: Ngủ bằng lệnh Assembly WFI (Wait For Interrupt) 
    */
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFI);   
}