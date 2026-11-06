/**
 * @file Mcal_Can.h
 * @brief MCAL CAN Header - Định nghĩa cấu trúc dữ liệu, trạng thái và API giao tiếp CAN phần cứng.
 */

#ifndef MCAL_CAN_H
#define MCAL_CAN_H

#include <stdint.h>

/** @brief Trạng thái hoạt động của trình điều khiển CAN */
typedef enum {
    MCAL_CAN_OK         = 0x00U,    /**< Gửi dữ liệu thành công */
    MCAL_CAN_ERROR      = 0x01U,    /**< Lỗi truyền phần cứng */
    MCAL_CAN_BUSY       = 0x02U     /**< Mailbox bận, chưa thể gửi */
} Mcal_Can_StatusType;

/** @brief Cấu trúc khung truyền/nhận dữ liệu CAN chuẩn (Standard ID) */
typedef struct {
    uint32_t StdId;     /**< ID chuẩn của khung CAN (Ví dụ: 0x300, 0x7E0, 0x7E8) */
    uint8_t Dlc;        /**< Độ dài dữ liệu (Data Length Code: 0 - 8 bytes) */
    uint8_t Data[8];    /**< Dữ liệu payload 8 bytes của khung CAN */
} Mcal_Can_FrameType;

/** @brief Kiểu con trỏ hàm Callback thông báo lên tầng EAL (CanIf) khi nhận được khung CAN từ ISR */
typedef void (*Mcal_Can_RxIndicationCbType)(const Mcal_Can_FrameType *pFrame);

/**
 * @brief Khởi tạo phần cứng CAN1, thiết lập bộ lọc Mask và bật ngắt nhận.
 */
void Mcal_Can_Init(void);

/**
 * @brief Đăng ký hàm Callback nhận dữ liệu CAN từ ngắt FIFO0 lên tầng trên (CanIf).
 * @param rxCb Con trỏ hàm xử lý tại tầng CanIf.
 */
void Mcal_Can_RegisterRxCallback(Mcal_Can_RxIndicationCbType rxCb);

/**
 * @brief Gửi một khung CAN ra bus thông qua Mailbox trống.
 * @param pFrame Con trỏ chứa thông tin khung CAN cần phát.
 * @return Mcal_Can_StatusType Trạng thái thực thi.
 */
Mcal_Can_StatusType Mcal_Can_Write(const Mcal_Can_FrameType *pFrame);

#endif /* MCAL_CAN_H */
