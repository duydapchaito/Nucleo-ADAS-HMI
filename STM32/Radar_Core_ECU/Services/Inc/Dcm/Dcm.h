/**
 * @file    Dcm.h
 * @brief   Cổng giao tiếp công khai (Public API) của module Diagnostic Communication Manager.
 */

#ifndef DCM_H
#define DCM_H

#include "Dcm_Types.h"
/**
 * @brief Khởi tạo toàn bộ các phân hệ DSL, DSD, DSP và nạp tham số mặc định từ EEPROM.
 */
void Dcm_Init(void);

/**
 * @brief Hàm chu kỳ của DCM (Gọi định kỳ để quản lý bộ đếm thời gian S3Server).
 * @param elapsedMs: Khoảng thời gian trôi qua giữa 2 lần gọi (ms).
 */
void Dcm_MainFunction(uint32_t elapsedMs);

void Dcm_ProvideRxPayload(const uint8_t *pReqPayload, uint16_t length);

#endif /* DCM_H */
