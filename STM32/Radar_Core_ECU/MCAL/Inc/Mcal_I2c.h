/**
 * @file    Mcal_I2c.h
 * @brief   Lớp trừu tượng phần cứng (MCAL) cho giao tiếp I2C, độc lập với RTOS qua Callback đồng bộ.
 */

#ifndef MCAL_I2C_H
#define MCAL_I2C_H

#include <stdint.h>
#include <stdbool.h>

/* Định nghĩa mã lỗi trả về chuẩn của tầng MCAL */
typedef enum {
    MCAL_I2C_OK         = 0x00U,
    MCAL_I2C_ERROR      = 0x01U,
    MCAL_I2C_BUSY       = 0x02U,
    MCAL_I2C_TIMEOUT    = 0x03U
} Mcal_I2c_StatusType;

/** @brief Bộ giao diện hàm đồng bộ OS tiêm (inject) xuống MCAL I2C lúc khởi tạo */
typedef struct {
    void (*ClearSync)(void);
    bool (*WaitSync)(uint32_t timeoutMs);
    void (*SignalFromIsr)(void);
} Mcal_I2c_SyncCallbacksType;

/**
 * @brief Đăng ký các hàm đồng bộ từ OS để MCAL I2C không phụ thuộc trực tiếp vào Os.h.
 */
void Mcal_I2c_RegisterSyncCallbacks(const Mcal_I2c_SyncCallbacksType *pCallbacks);

/**
 * @brief  Ghi một mảng dữ liệu vào bộ nhớ I2C (EEPROM).
 * @param  DevAddress: Địa chỉ 7-bit của thiết bị (đã dịch trái 1 bit, ví dụ 0xA0).
 * @param  MemAddress: Địa chỉ ô nhớ nội bộ bên trong chip cần ghi.
 * @param  MemAddSize: Kích thước của địa chỉ ô nhớ (1 byte hoặc 2 bytes).
 * @param  pData: Con trỏ trỏ tới mảng dữ liệu cần ghi.
 * @param  Size: Số lượng byte dữ liệu cần ghi.
 * @param  Timeout: Thời gian chờ tối đa (ms).
 * @retval Mcal_I2c_StatusType (OK, ERROR, BUSY, TIMEOUT)
 */
Mcal_I2c_StatusType Mcal_I2c_WriteMem(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief  Đọc một mảng dữ liệu từ bộ nhớ I2C (EEPROM).
 * @param  DevAddress: Địa chỉ 7-bit của thiết bị (đã dịch trái 1 bit).
 * @param  MemAddress: Địa chỉ ô nhớ nội bộ bên trong chip cần đọc.
 * @param  MemAddSize: Kích thước của địa chỉ ô nhớ.
 * @param  pData: Con trỏ chứa dữ liệu đọc về.
 * @param  Size: Số lượng byte dữ liệu cần đọc.
 * @param  Timeout: Thời gian chờ tối đa (ms).
 * @retval Mcal_I2c_StatusType (OK, ERROR, BUSY, TIMEOUT)
 */
Mcal_I2c_StatusType Mcal_I2c_ReadMem(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t* pData, uint16_t Size, uint32_t Timeout);

/**
 * @brief  Kiểm tra xem thiết bị I2C đã sẵn sàng giao tiếp chưa.
 * @note   Cực kỳ quan trọng để polling (chờ) EEPROM hoàn thành chu kỳ ghi (Page Write Cycle).
 * @param  DevAddress: Địa chỉ thiết bị.
 * @param  Trials: Số lần thử Ping thiết bị.
 * @param  Timeout: Thời gian chờ cho mỗi lần thử.
 * @retval Mcal_I2c_StatusType (OK nếu sẵn sàng)
 */
 Mcal_I2c_StatusType Mcal_I2c_IsDeviceReady(uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);

#endif /* MCAL_I2C_H */
