/**
 * @file Mcal_Pwr.h
 * @brief MCAL Power & MCU Reset Header - Quản lý chế độ năng lượng và khởi động lại vi điều khiển.
 */

#ifndef MCAL_PWR_H
#define MCAL_PWR_H

/**
 * @brief Đưa vi điều khiển vào chế độ Sleep sử dụng lệnh WFI (Wait For Interrupt).
 */
void Mcal_Pwr_EnterSleepWFI(void); 

/**
 * @brief Khởi động lại toàn bộ vi điều khiển (Cold/Hard System Reset qua NVIC).
 */
void Mcal_Pwr_SystemReset(void);

#endif /* MCAL_PWR_H */