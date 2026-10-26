/**
 * @file Cdd_Fusion.h
 * @brief CDD Math Fusion Header - Định nghĩa API hợp thể dữ liệu cảm biến radar.
 */

#ifndef CDD_FUSION_H
#define CDD_FUSION_H

#include <stdint.h>

#define MATH_HW_SENSORS_COUNT       4U      /**< Số lượng cảm biến phần cứng vật lý thực tế */
#define MATH_LOGIC_SENSORS_COUNT    6U      /**< Số lượng cảm biến logic hiển thị trên HMI (Bao gồm mắt ảo) */

/**
 * @brief Thuật toán nội suy khoảng cách mắt trung tâm (Virtual Center).
 */
uint16_t Math_Fusion_VirtualCenter(uint16_t dist_left, uint16_t dist_right);

/**
 * @brief Khởi tạo các bộ lọc cho toàn bộ cảm biến phần cứng.
 */
void Math_Fusion_Init(uint16_t min_limit, uint16_t max_limit);

/**
 * @brief Pipeline tổng hợp: Biến đổi 4 ticks thô thành 6 khoảng cách logic và byte lỗi.
 */
void Math_Fusion_ProcessRadarArray(const uint32_t raw_ticks_4[MATH_HW_SENSORS_COUNT],
                                    uint16_t min_limit, uint16_t max_limit,
                                    uint16_t out_dist_6[MATH_LOGIC_SENSORS_COUNT],
                                    uint8_t *out_err_mask_6);

#endif /* CDD_FUSION_H */
