/**
 * @file Eal_CanIf.c
 * @brief EAL CAN Interface - Trạm trừu tượng hóa phần cứng CAN và định tuyến tĩnh lên CanTp, CanNm, Com.
 */

#include "Eal_CanIf.h"
#include "Mcal_Can.h"
#include "CanTp/CanTp.h"
#include "CanNm/CanNm.h"
#include "Com_Manager.h"
#include "Os.h"
#include "Log.h"
#include <string.h>


static const uint32_t CanIf_TxPduToCanId[CANIF_TX_PDU_MAX] = {
    CANIF_TX_COM_STATUS_ID, /* CANIF_TX_PDU_RADAR_STATUS (0x300) */
    CANIF_TX_UDS_RESP_ID,   /* CANIF_TX_PDU_UDS_RESP     (0x7E8) */
    CANIF_TX_NM_NODE_ID     /* CANIF_TX_PDU_NM_NODE      (0x501) */
};

static void Eal_CanIf_McalRxIsrCallback(const Mcal_Can_FrameType *pFrame)
{
    if (pFrame != NULL) {
        Os_CanRx_PushFromISR(pFrame->StdId, pFrame->Dlc, pFrame->Data);
    }
}

void Eal_CanIf_Init(void)
{
    Mcal_Can_RegisterRxCallback(Eal_CanIf_McalRxIsrCallback);
    Mcal_Can_Init();
}

uint8_t Eal_CanIf_Transmit(CanIf_TxPduIdType TxPduId, const CanIf_PduInfoType *PduInfoPtr)
{
    if ((TxPduId >= CANIF_TX_PDU_MAX) || (PduInfoPtr == NULL) || 
        (PduInfoPtr->SduDataPtr == NULL) || (PduInfoPtr->SduLength > 8U)) {
        return 2U;
    }

    Mcal_Can_FrameType frame = {0};
    frame.StdId = CanIf_TxPduToCanId[TxPduId];
    frame.Dlc = PduInfoPtr->SduLength;
    memcpy(frame.Data, PduInfoPtr->SduDataPtr, PduInfoPtr->SduLength);

    Mcal_Can_StatusType status = Mcal_Can_Write(&frame);

    if (frame.StdId == CANIF_TX_UDS_RESP_ID) {
        LOG_I("CAN_TX: ID=0x%03lX, Status=%u", (unsigned long)frame.StdId, status);
    }

    if (status == MCAL_CAN_OK)      return 0U;
    if (status == MCAL_CAN_BUSY)    return 1U;
    return 2U;
}

/**
 * @brief Xử lý chỉ thị nhận dữ liệu CAN từ tầng dưới và in log hệ thống.
 */
void Eal_CanIf_RxIndication(uint32_t CanId, uint8_t Dlc, const uint8_t *pData)
{
    if ((pData == NULL) || (Dlc == 0U) || (Dlc > 8U)) {
        return;
    }

    LOG_I("CAN_RX: ID=0x%03lX, DLC=%u, Data=[%02X %02X %02X %02X...]",
        (unsigned long)CanId, Dlc, pData[0], pData[1], pData[2], pData[3]);

    CanIf_PduInfoType pduInfo = {
        .SduDataPtr = pData,
        .SduLength  = Dlc
    };

    /* 1. Định tuyến nhánh Chẩn đoán (CanTp) */
    if (CanId == CANIF_RX_UDS_PHYS_ID) {
        CanTp_RxIndication(CANIF_RX_PDU_UDS_PHYS, &pduInfo);
    } else if (CanId == CANIF_RX_UDS_FUNC_ID) {
        CanTp_RxIndication(CANIF_RX_PDU_UDS_FUNC, &pduInfo);
    }

    /* 2. Định tuyến nhánh Quản lý mạng (CanNm) */
    else if ((CanId >= CANIF_RX_NM_BASE_ID) && (CanId <= CANIF_RX_NM_END_ID)) {
        CanNm_RxIndication(CANIF_RX_PDU_NM_MSG, &pduInfo);
    }

    /* 3. Định tuyến nhánh Dữ liệu vận hành xe (Com_Manager) */
    else if (CanId == CANIF_RX_COM_VEH_ID) {
        Com_Manager_RxIndication(CANIF_RX_PDU_COM_VEH_DYN, &pduInfo);
    } else {
        LOG_W("CanIf RX Unhandled [0x%03lX] DLC:%u", (unsigned long)CanId, Dlc);        
    }
}
