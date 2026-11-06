/**
 * @file Com_Manager.h
 * @brief Services Com Manager Header - Định nghĩa cấu trúc giao tiếp truyền tải dữ liệu hệ thống.
 */

#ifndef COM_MANAGER_H
#define COM_MANAGER_H

#include <stdint.h>
#include "Eal_CanIf.h"

/** @brief Trạng thái truyền thông điệp của Com Manager */
typedef enum {
    COM_TX_OK       = 0U,   /**< Truyền thông điệp thành công */
    COM_TX_BUSY     = 1U,   /**< Đường truyền bận */
    COM_TX_ERROR    = 2U    /**< Lỗi truyền dữ liệu */
} Com_StatusType;

/**
 * @brief Đóng gói và gửi khung CAN định danh 0x300 (8 Bytes) chuẩn AUTOSAR:
 *        - Byte 0: GearState (0:P, 1:R, 2:N, 3:D)
 *        - Byte 1..6: FL, FC, FR, RL, RC, RR (Scale 0.5x, 1 LSB = 2 cm)
 *        - Byte 7: Error Bitmask (Bit 0..5: Lỗi 6 cảm biến, Bit 6..7: Chẩn đoán cần số)
 */
Com_StatusType Com_Manager_SendRadarCan(uint8_t GearState, const uint16_t Dist6[6], uint8_t ErrorMask);
    
/**
 * @brief Gửi chuỗi log NMEA qua đường UART2 sử dụng DMA.
 */
Com_StatusType Com_Manager_SendRadarNmea(char GearChar, const uint16_t Dist6[6]);

/* Callback từ Eal_CanIf khi nhận được gói tin COM động lực học xe (ID 0x200) */
void Com_Manager_RxIndication(CanIf_RxPduIdType RxPduId, const CanIf_PduInfoType *PduInfoPtr);

#endif /* COM_MANAGER_H */
