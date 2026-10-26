/**
 * @file Mcal_Timer.h
 * @brief MCAL Timer Header - Định nghĩa API quản lý Input Capture, Encoder phần cứng và hàm trễ.
 */

#ifndef MCAL_TIMER_H
#define MCAL_TIMER_H

#include <stdint.h>

/**
 * @brief Khởi tạo các bộ định thời (TIM1 cho siêu âm, TIM2 cho Encoder).
 */
void Mcal_Timer_Init(void);

/**
 * @brief Tạo khoảng trễ chính xác theo đơn vị micro-giây.
 * @param us Số micro-giây cần trễ (dùng cho xung Trigger 10us).
 */
void Mcal_Timer_DelayUs(uint16_t us);

/**
 * @brief Đọc giá trị xung đếm hiện tại từ bộ Quadrature Encoder (cần số).
 * @return uint32_t Giá trị đếm thô của Encoder.
 */
uint32_t Mcal_Timer_GetEncoderCount(void);

/**
 * @brief Bật ngắt Input Capture cho kênh Timer tương ứng.
 * @param Channel Chỉ số kênh (0 đến 3).
 */
void Mcal_Timer_StartCaptureChannel(uint8_t Channel);

/**
 * @brief Tắt ngắt Input Capture của kênh Timer.
 * @param Channel Chỉ số kênh (0 đến 3).
 */
void Mcal_Timer_StopCaptureChannel(uint8_t Channel);

/**
 * @brief Cấu hình sườn xung bắt tín hiệu cho ngắt Input Capture.
 * @param Channel Chỉ số kênh (0 đến 3).
 * @param isRisingEdge 1: Bắt cạnh lên (Rising), 0: Bắt cạnh xuống (Falling).
 */
void Mcal_Timer_SetCapturePolarity(uint8_t Channel, uint8_t isRisingEdge);

#endif /* MCAL_TIMER_H */
