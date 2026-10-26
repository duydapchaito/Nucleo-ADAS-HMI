/**
 * @file Eal_CanIf.h
 * @brief EAL CAN Interface Header - Cấu trúc PDU và các API giao tiếp mạng CAN chuẩn AUTOSAR.
 */

#ifndef EAL_CANIF_H
#define EAL_CANIF_H

#include <stdint.h>

/** @brief Định danh các PDU truyền đi trên hệ thống mạng CAN */
typedef enum {
    CANIF_TX_PDU_RADAR_STATUS = 0,  /* CAN ID 0x300 */
    CANIF_TX_PDU_DIAG_RESP,         /* CAN ID 0x7E8 (Dành sẵn cho UDS) */
    CANIF_TX_PDU_MAX
} CanIf_TxPduIdType;

/** @brief Cấu trúc thông tin gói dữ liệu PDU (Protocol Data Unit) */
typedef struct {
    const uint8_t *SduDataPtr;
    uint8_t SduLength;
} CanIf_PduInfoType;

/**
 * @brief Khởi tạo hệ thống CAN Interface.
 */
void Eal_CanIf_Init(void);

/**
 * @brief Truyền một PDU qua bus CAN.
 */
uint8_t Eal_CanIf_Transmit(CanIf_TxPduIdType TxPduId, const CanIf_PduInfoType *PduInfoPtr);

/**
 * @brief Nhận chỉ thị có khung CAN đến từ phần cứng.
 */
void Eal_CanIf_RxIndication(uint32_t CanId, uint8_t Dlc, const uint8_t *pData);

#endif /* EAL_CANIF_H */
