/**
 * @file Mcal_Pwr.h
 * @brief MCAL Power Header - Quản lý chế độ năng lượng và tiết kiệm điện của vi điều khiển.
 */

#ifndef MCAL_PWR_H
#define MCAL_PWR_H

/**
 * @brief Đưa vi điều khiển vào chế độ Sleep sử dụng lệnh WFI (Wait For Interrupt).
 * @note Ngoại vi vẫn chạy ngầm, CPU tự thức dậy ngay khi có ngắt phần cứng.
 */
void Mcal_Pwr_EnterSleepWFI(void); 

#endif /* MCAL_PWR_H */