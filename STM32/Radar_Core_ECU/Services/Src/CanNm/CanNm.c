/**
 * @file CanNm.c
 * @brief Triển khai máy trạng thái AUTOSAR CAN Network Management.
 */

#include "CanNm/CanNm.h"
#include "Log.h"

#include <stddef.h>

static CanNm_StateType  s_NmState           = CANNM_STATE_REPEAT_MESSAGE;
static bool             s_NetworkRequested  = true; /* Mặc định ECU active khi khởi động */
static uint32_t         s_StateTimerMs      = CANNM_REPEAT_MSG_TIME_MS;
static uint32_t         s_TimeoutTimerMs    = CANNM_TIMEOUT_TIME_MS;
static uint32_t         s_MsgCycleTimerMs   = 0U;

static void CanNm_SendNmPdu(void)
{
    uint8_t nmPayload[8] = {
        CANNM_NODE_ID,                      /* Byte 0: Source Node Identifier */
        (uint8_t)s_NmState,                 /* Byte 1: Current CanNm State */
        s_NetworkRequested ? 0x01U : 0x00U, /* Byte 2: Control Bit Vector / Active Wakeup */
        0x00U, 0x00U, 0x00U, 0x00U, 0x00U
    };

    CanIf_PduInfoType pdu = {
        .SduDataPtr = nmPayload,
        .SduLength = 8U
    };

    (void)Eal_CanIf_Transmit(CANIF_TX_PDU_NM_NODE, &pdu);
}

void CanNm_Init(void)
{
    s_NmState           = CANNM_STATE_REPEAT_MESSAGE;
    s_NetworkRequested  = true;
    s_StateTimerMs      = CANNM_REPEAT_MSG_TIME_MS;
    s_TimeoutTimerMs    = CANNM_TIMEOUT_TIME_MS;
    s_MsgCycleTimerMs   = CANNM_MSG_CYCLE_MS;
}

void CanNm_NetworkRequest(void)
{
    s_NetworkRequested  = true;
    if ((s_NmState == CANNM_STATE_BUS_SLEEP) || (s_NmState == CANNM_STATE_PREPARE_BUS_SLEEP)) {
        s_NmState           = CANNM_STATE_REPEAT_MESSAGE;
        s_StateTimerMs      = CANNM_REPEAT_MSG_TIME_MS;
        s_TimeoutTimerMs    = CANNM_TIMEOUT_TIME_MS;
        LOG_I("CanNm: Wakeup -> REPEAT_MESSAGE");
    } else if (s_NmState == CANNM_STATE_READY_SLEEP) {
        s_NmState = CANNM_STATE_NORMAL_OPERATION;
    }
}

void CanNm_NetworkRelease(void)
{
    s_NetworkRequested = false;
}

CanNm_StateType CanNm_GetState(void)
{
    return s_NmState;
}

void CanNm_RxIndication(CanIf_RxPduIdType RxPduId, const CanIf_PduInfoType *PduInfoPtr)
{
    (void)RxPduId;
    if ((PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL) || (PduInfoPtr->SduLength == 0U)) {
        return;
    }

    /* Làm mới NM Timeout mỗi khi thấy bất kỳ gói NM nào trên bus */
    s_TimeoutTimerMs = CANNM_TIMEOUT_TIME_MS;

    /* Nếu đang ở chế độ ngủ hoặc chuẩn bị ngủ mà nhận được gói NM -> Thức dậy */
    if ((s_NmState == CANNM_STATE_BUS_SLEEP) || (s_NmState == CANNM_STATE_PREPARE_BUS_SLEEP)) {
        s_NmState = CANNM_STATE_REPEAT_MESSAGE;
        s_StateTimerMs = CANNM_REPEAT_MSG_TIME_MS;
        LOG_I("CanNm: Bus Activity Detected -> REPEAT_MESSAGE");
    }
}

void CanNm_MainFunction(uint32_t elapsedMs)
{
    switch (s_NmState) {
        case CANNM_STATE_REPEAT_MESSAGE:
            if (s_StateTimerMs <= elapsedMs) {
                s_StateTimerMs = 0U;
                s_NmState = s_NetworkRequested ? CANNM_STATE_NORMAL_OPERATION : CANNM_STATE_READY_SLEEP;
            } else {
                s_StateTimerMs -= elapsedMs;
            }
            break;

        case CANNM_STATE_NORMAL_OPERATION:
            if (!s_NetworkRequested) {
                s_NmState = CANNM_STATE_READY_SLEEP;
                s_TimeoutTimerMs = CANNM_TIMEOUT_TIME_MS;
            } else {
                /* Duy trì chu kỳ gói tin NM */
                if (s_MsgCycleTimerMs <= elapsedMs) {
                    s_MsgCycleTimerMs = CANNM_MSG_CYCLE_MS;
                    /* Có thể bật CanNm_SendNmPdu() nếu muốn phát 0x501 định kỳ */
                    CanNm_SendNmPdu();
                } else {
                    s_MsgCycleTimerMs -= elapsedMs;
                }
            }
            break;

        case CANNM_STATE_READY_SLEEP:
            if (s_NetworkRequested) {
                s_NmState = CANNM_STATE_NORMAL_OPERATION;
            } else if (s_TimeoutTimerMs <= elapsedMs) {
                s_NmState = CANNM_STATE_PREPARE_BUS_SLEEP;
                s_StateTimerMs = CANNM_WAIT_BUS_SLEEP_TIME_MS;
                LOG_W("CanNm: NM Timeout -> PREPARE_BUS_SLEEP");
            } else {
                s_TimeoutTimerMs -= elapsedMs;
            }
            break;

        case CANNM_STATE_PREPARE_BUS_SLEEP:
            if (s_NetworkRequested) {
                s_NmState = CANNM_STATE_REPEAT_MESSAGE;
                s_StateTimerMs = CANNM_REPEAT_MSG_TIME_MS;
            } else if (s_StateTimerMs <= elapsedMs) {
                s_NmState = CANNM_STATE_BUS_SLEEP;
                LOG_W("CanNm: Entered BUS_SLEEP Mode");
            } else {
                s_TimeoutTimerMs -= elapsedMs;
            }
            break;

        case CANNM_STATE_BUS_SLEEP:
        default:
            break;
    }
}
