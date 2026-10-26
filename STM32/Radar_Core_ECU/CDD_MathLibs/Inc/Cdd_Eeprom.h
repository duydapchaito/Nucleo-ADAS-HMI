/**
 * @file    Cdd_Eeprom.h
 * @brief   Complex Device Driver (CDD) cho bộ nhớ EEPROM ngoại vi AT24C02.
 * @details Cung cấp giao diện đọc/ghi trừu tượng hóa phân trang cho các dịch vụ
 *          lưu trữ cấu hình ECU, thông số chẩn đoán (DCM) và sự kiện lỗi (DEM).
 * @version 1.0
 */

#ifndef CDD_EEPROM_H
#define CDD_EEPROM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ==================================================================== */
/* THÔNG SỐ ĐẶC TÍNH VẬT LÝ CỦA CHIP EEPROM AT24C02                     */
/* ==================================================================== */

/** @brief Địa chỉ bus I2C 8-bit của AT24C02 (Địa chỉ 7-bit 0x50 dịch trái 1 bit, chân A0=A1=A2=GND) */
#define CDD_EEPROM_I2C_ADDR         0xA0U

/** @brief Kích thước một trang nhớ vật lý (Page size: tối đa 8 byte cho mỗi chu kỳ ghi) */
#define CDD_EEPROM_PAGE_SIZE        8U

/** @brief Tổng dung lượng định danh của AT24C02 (2 Kbits = 256 Bytes: 0x00 đến 0xFF) */
#define CDD_EEPROM_TOTAL_SIZE       256U

/* ==================================================================== */
/* SƠ ĐỒ PHÂN BỔ BỘ NHỚ LƯU TRỮ NVM (EEPROM MEMORY MAP)                 */
/* ==================================================================== */

/** @brief Địa chỉ lưu trữ tham số mặc định phục vụ kiểm tra UDS DID 0x22 / 0x2E (4 Bytes) */
#define NVM_ADDR_DEFAULT_PARAM      0x00U

/** @brief Địa chỉ lưu bộ đếm số lần nhập sai khóa bảo mật UDS SID 0x27 (Anti-Brute Force Counter) */
#define NVM_ADDR_SECURITY_ATTEMPTS  0x10U

/** @brief Địa chỉ bắt đầu lưu bảng ghi sự kiện lỗi và dữ liệu đóng băng DTC của DEM (Freeze Frames) */
#define NVM_ADDR_DTC_START_BLOCK    0x20U

/* ==================================================================== */
/* ĐỊNH NGHĨA KIỂU DỮ LIỆU VÀ TRẠNG THÁI TRẢ VỀ                         */
/* ==================================================================== */

/**
 * @brief Mã trạng thái trả về của module CDD EEPROM.
 */
typedef enum {
    CDD_EEPROM_OK           = 0x00U,    /**< Thao tác thành công hoàn toàn */
    CDD_EEPROM_ERR_I2C      = 0x01U,    /**< Lỗi bus I2C, timeout hoặc thiết bị không phản hồi ACK */
    CDD_EEPROM_ERR_PARAM    = 0x10U     /**< Tham số không hợp lệ (Vượt quá dung lượng 256B, NULL pointer) */
} Cdd_Eeprom_StatusType;

/* ==================================================================== */
/* CÁC NGUYÊN MẪU HÀM GIAO DIỆN CỦA CDD EEPROM                          */
/* ==================================================================== */

/**
 * @brief  Khởi tạo và kiểm tra trạng thái vật lý của chip EEPROM trên bus I2C.
 * @retval CDD_EEPROM_OK nếu thiết bị phản hồi ACK, ngược lại trả về CDD_EEPROM_ERR_I2C.
 */
Cdd_Eeprom_StatusType Cdd_Eeprom_Init(void);

/**
 * @brief  Ghi một chuỗi byte liên tục có độ dài bất kỳ vào EEPROM.
 * @note   Tự động tính toán ranh giới trang (8-byte page boundary) và phân rã thành
 *         nhiều lần ghi để tránh lỗi Page Roll-over làm ghi đè mất dữ liệu.
 *         Task gọi hàm sẽ tự động nhường CPU trong thời gian chờ phần cứng ghi (~5ms).
 * @param  MemAddr: Địa chỉ ô nhớ bắt đầu ghi bên trong EEPROM (0x00 - 0xFF).
 * @param  pData:   Con trỏ trỏ đến mảng dữ liệu nguồn cần ghi.
 * @param  Len:     Số lượng byte cần ghi.
 * @retval Cdd_Eeprom_StatusType: Mã trạng thái thực thi.
 */
Cdd_Eeprom_StatusType Cdd_Eeprom_WriteBuffer(uint8_t MemAddr, const uint8_t *pData, uint16_t Len);

/**
 * @brief  Đọc một chuỗi byte liên tục từ EEPROM.
 * @note   Tận dụng cơ chế đọc tuần tự (Sequential Read) của phần cứng AT24C02,
 *         không bị giới hạn kích thước theo trang như thao tác ghi.
 * @param  MemAddr: Địa chỉ ô nhớ bắt đầu đọc bên trong EEPROM (0x00 - 0xFF).
 * @param  pData:   Con trỏ trỏ đến bộ đệm đích chứa dữ liệu đọc về.
 * @param  Len:     Số lượng byte cần đọc.
 * @retval Cdd_Eeprom_StatusType: Mã trạng thái thực thi.
 */
Cdd_Eeprom_StatusType Cdd_Eeprom_ReadBuffer(uint8_t MemAddr, uint8_t *pData, uint16_t Len);

#endif /* CDD_EEPROM_H */
