/**
 * @file Eal_CanIf.c
 * @brief EAL CAN Interface - Đóng gói PDU ID thành khung CAN vật lý và xử lý chỉ thị nhận.
 */

#include "Eal_CanIf.h"
#include "Mcal_Can.h"
#include "Os.h"
#include <string.h>

// Bảng tra cứu ánh xạ PDU ID sang CAN Identifier tương ứng
static const uint32_t CanIf_TxPduToCanId[CANIF_TX_PDU_MAX] = {
    0x300U, /* CANIF_TX_PDU_RADAR_STATUS: Khung trạng thái 6 cảm biến và cần số */
    0x7E8U  /* CANIF_TX_PDU_DIAG_RESP: Khung phản hồi chẩn đoán UDS (Reserved) */
};

/**
 * @brief Khởi tạo giao diện CAN Interface thông qua tầng MCAL.
 */
void Eal_CanIf_Init(void)
{
    Mcal_Can_Init();
}

/**
 * @brief Gửi một PDU qua mạng CAN dựa vào định danh PDU ID.
 * @param TxPduId Định danh PDU truyền đi.
 * @param PduInfoPtr Con trỏ chứa payload dữ liệu và độ dài DLC.
 * @return uint8_t 0: Thành công (OK), 1: Bận (BUSY), 2: Lỗi (ERROR).
 */
uint8_t Eal_CanIf_Transmit(CanIf_TxPduIdType TxPduId, const CanIf_PduInfoType *PduInfoPtr)
{
    if (TxPduId >= CANIF_TX_PDU_MAX || PduInfoPtr == NULL || PduInfoPtr->SduLength > 8U) {
        return 2U;
    }

    Mcal_Can_FrameType frame = {0};
    frame.StdId = CanIf_TxPduToCanId[TxPduId];
    frame.Dlc = PduInfoPtr->SduLength;
    memcpy(frame.Data, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);

    Mcal_Can_StatusType status = Mcal_Can_Write(&frame);
    if (status == MCAL_CAN_OK) return 0U;
    if (status == MCAL_CAN_BUSY) return 1U;
    return 2U;
}

/**
 * @brief Xử lý chỉ thị nhận dữ liệu CAN từ tầng dưới và in log hệ thống.
 */
void Eal_CanIf_RxIndication(uint32_t CanId, uint8_t Dlc, const uint8_t *pData)
{
    OS_LOG_I("CanIf RX [0x%03lX] DLC:%u Data:%02X %02X %02X %02X",
            CanId, Dlc, pData[0], pData[1], pData[2], pData[3]);
}
