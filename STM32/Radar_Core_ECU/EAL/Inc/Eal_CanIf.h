/**
 * @file Eal_CanIf.h
 * @brief EAL CAN Interface Header - Trừu tượng hóa Driver CAN và định tuyến L-PDU (Lightweight PduR).
 */

#ifndef EAL_CANIF_H
#define EAL_CANIF_H

#include <stdint.h>

/* ==================================================================== */
/* CẤU HÌNH BẢNG ĐỊA CHỈ CAN ID VẬT LÝ TRÊN MẠNG                        */
/* ==================================================================== */
#define CANIF_RX_COM_VEH_ID     0x200U  /**< CAN ID nhận Telemetry động lực học xe (Speed, Brake, Batt) */
#define CANIF_TX_COM_STATUS_ID  0x300U  /**< CAN ID phát trạng thái Radar định kỳ (COM) */

#define CANIF_RX_NM_BASE_ID     0x500U  /**< Dải CAN ID bắt đầu của Network Management (0x500..0x5FF) */
#define CANIF_RX_NM_END_ID      0x5FFU  /**< Dải CAN ID kết thúc của Network Management */
#define CANIF_TX_NM_NODE_ID     0x501U  /**< CAN ID phát bản tin CanNm của Radar ECU */

#define CANIF_RX_UDS_FUNC_ID    0x7DFU  /**< CAN ID yêu cầu chẩn đoán quảng bá (Functional) */
#define CANIF_RX_UDS_PHYS_ID    0x7E0U  /**< CAN ID yêu cầu chẩn đoán vật lý (Tester -> ECU) */
#define CANIF_TX_UDS_RESP_ID    0x7E8U  /**< CAN ID phản hồi chẩn đoán (ECU -> Tester) */

/** @brief Danh mục các PDU truyền đi (Tx PDU IDs) */
typedef enum {
    CANIF_TX_PDU_RADAR_STATUS   = 0U,   /**< 0x300: Phục vụ Com_Manager */
    CANIF_TX_PDU_UDS_RESP       = 1U,   /**< 0x7E8: Phục vụ CanTp (UDS) */
    CANIF_TX_PDU_NM_NODE        = 2U,   /**< 0x501: Phục vụ CanNm */
    CANIF_TX_PDU_MAX            = 3U
} CanIf_TxPduIdType;

/** @brief Danh mục các PDU nhận vào (Rx PDU IDs) */
typedef enum {
    CANIF_RX_PDU_UDS_PHYS       = 0U,   /**< 0x7E0 -> Định tuyến lên CanTp */
    CANIF_RX_PDU_UDS_FUNC       = 1U,   /**< 0x7DF -> Định tuyến lên CanTp */
    CANIF_RX_PDU_NM_MSG         = 2U,   /**< 0x500..0x5FF -> Định tuyến lên CanNm */
    CANIF_RX_PDU_COM_VEH_DYN    = 3U,   /**< 0x200 -> Định tuyến lên Com_Manager */
    CANIF_RX_PDU_MAX            = 4U
} CanIf_RxPduIdType;

/** @brief Cấu trúc thông tin gói dữ liệu PDU (Protocol Data Unit) */
typedef struct {
    const uint8_t   *SduDataPtr;
    uint8_t         SduLength;
} CanIf_PduInfoType;


void    Eal_CanIf_Init(void);
uint8_t Eal_CanIf_Transmit(CanIf_TxPduIdType TxPduId, const CanIf_PduInfoType *PduInfoPtr);
void    Eal_CanIf_RxIndication(uint32_t CanId, uint8_t Dlc, const uint8_t *pData);

#endif /* EAL_CANIF_H */
