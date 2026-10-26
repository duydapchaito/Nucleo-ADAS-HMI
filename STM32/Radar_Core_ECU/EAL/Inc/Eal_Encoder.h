/**
 * @file Eal_Encoder.h
 * @brief EAL Encoder Header - Định nghĩa API đọc bước chuyển cần số từ Encoder.
 */

#ifndef EAL_ENCODER_H
#define EAL_ENCODER_H

#include <stdint.h>

/**
 * @brief Khởi tạo trạng thái ban đầu của Encoder.
 */
void Eal_Encoder_Init(void);

/**
 * @brief Lấy delta thay đổi nấc số của Encoder.
 * @return int32_t 1 (Tăng), -1 (Giảm), 0 (Không đổi).
 */
int32_t Eal_Encoder_GetDelta(void);

#endif /* EAL_ENCODER_H */
