/**
 * @file Mcal_Can.c
 * @brief MCAL CAN Driver - Cấu hình bộ lọc, gửi frame và ủy quyền ngắt nhận CAN lên tầng EAL.
 */

#include "main.h"
#include "can.h"
#include "Mcal_Can.h"
#include <stddef.h>

extern CAN_HandleTypeDef hcan1;

static Mcal_Can_RxIndicationCbType s_CanRxCallback = NULL;

/**
 * @brief Khởi tạo phần cứng CAN1, thiết lập bộ lọc Mask 32-bit và bật ngắt FIFO0.
 */
void Mcal_Can_Init(void)
{
    CAN_FilterTypeDef sFilter = {0};

    sFilter.FilterBank              = 0U;
    sFilter.FilterMode              = CAN_FILTERMODE_IDMASK;
    sFilter.FilterScale             = CAN_FILTERSCALE_32BIT;
    sFilter.FilterIdHigh            = 0x0000U;
    sFilter.FilterIdLow             = 0x0000U;
    sFilter.FilterMaskIdHigh        = 0x0000U;
    sFilter.FilterMaskIdLow         = 0x0000U;
    sFilter.FilterFIFOAssignment    = CAN_RX_FIFO0;
    sFilter.FilterActivation        = CAN_FILTER_ENABLE;
    sFilter.SlaveStartFilterBank    = 14U;

    (void)HAL_CAN_ConfigFilter(&hcan1, &sFilter);
    (void)HAL_CAN_Start(&hcan1);
    (void)HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}

void Mcal_Can_RegisterRxCallback(Mcal_Can_RxIndicationCbType rxCb)
{
    s_CanRxCallback = rxCb;
}

/**
 * @brief Gửi một khung CAN ra bus thông qua Mailbox trống.
 * @param pFrame Con trỏ chứa thông tin khung CAN cần phát.
 * @return Mcal_Can_StatusType Trạng thái gửi (OK / BUSY / ERROR).
 */
Mcal_Can_StatusType Mcal_Can_Write(const Mcal_Can_FrameType *pFrame)
{
    if ((pFrame == NULL) || (pFrame->Dlc > 8U)) {
        return MCAL_CAN_ERROR;
    }

    if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) == 0U) {
        return MCAL_CAN_BUSY;
    }

    CAN_TxHeaderTypeDef txHeader = {0};
    uint32_t txMailbox = 0U;

    txHeader.StdId              = pFrame->StdId;
    txHeader.IDE                = CAN_ID_STD;
    txHeader.RTR                = CAN_RTR_DATA;
    txHeader.DLC                = pFrame->Dlc;
    txHeader.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, (uint8_t *)pFrame->Data, &txMailbox) == HAL_OK) {
        return MCAL_CAN_OK;
    }
    return MCAL_CAN_ERROR;
}

/**
 * @brief Callback ngắt phần cứng: Khi có gói tin CAN đổ vào FIFO0, bốc ra và đẩy vào OS Queue.
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    if (hcan->Instance == CAN1) {
        CAN_RxHeaderTypeDef rxHeader;
        Mcal_Can_FrameType rxFrame;

        if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxFrame.Data) == HAL_OK) {
            rxFrame.StdId = rxHeader.StdId;
            rxFrame.Dlc = (uint8_t)rxHeader.DLC;
            if (s_CanRxCallback != NULL) {
                s_CanRxCallback(&rxFrame);
            }
        }
    }
}
