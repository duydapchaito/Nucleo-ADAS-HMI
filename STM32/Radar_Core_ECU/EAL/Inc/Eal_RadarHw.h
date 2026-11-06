/**
 * @file Eal_RadarHw.h
 * @brief EAL Radar Hardware Header - Quản lý phát xung Trigger và thu thập Echo cho 4 cảm biến siêu âm.
 */

#ifndef EAL_RADARHW_H
#define EAL_RADARHW_H

#include <stdint.h>
#include "Mcal_Dio.h"

/** @brief Cấu trúc lưu trữ dữ liệu thô thu được từ ngắt Input Capture của từng cảm biến */
typedef struct {
    volatile uint32_t   Capture_Val1;
    volatile uint32_t   Capture_Val2;
    volatile uint8_t    Is_First_Edge;
    volatile uint8_t    Is_Data_Ready;
    volatile uint32_t   Delta_Ticks; // Dữ liệu quý giá nhất
} Eal_SensorRaw_t;

// Thông số kỹ thuật chuẩn của cảm biến siêu âm HC-SR04
#define EAL_RADAR_MIN_RANGE_CM  2U      /**< Khoảng cách đo tối thiểu (cm) */
#define EAL_RADAR_MAX_RANGE_CM  400U    /**< Khoảng cách đo tối đa (cm) */

/**
 * @brief Khởi tạo phần cứng Timer và đăng ký Callback Input Capture cho 4 mắt siêu âm.
 */
void Eal_Radar_Init(void);

/**
 * @brief Kích hoạt xung Trigger cho cảm biến siêu âm và đặt lại trạng thái chờ Echo mới.
 */
void Eal_Radar_Trigger(uint8_t SensorId);

/**
 * @brief Xử lý giá trị capture từ ngắt phần cứng.
 */
void Eal_Radar_ProcessCapture(uint8_t SensorId, uint32_t CapturedValue);

/**
 * @brief Lấy số ticks thô của cảm biến (Trả về 0 nếu quá hạn/mất xung Echo).
 */
uint32_t Eal_Radar_GetRawTicks(uint8_t SensorId);

#endif /* EAL_RADARHW_H */
