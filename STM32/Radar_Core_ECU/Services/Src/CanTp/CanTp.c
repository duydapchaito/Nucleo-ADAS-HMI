/**
 * @file CanTp.c
 * @brief Triển khai cỗ máy trạng thái CanTp theo chuẩn ISO 15765-2.
 */

#include "CanTp/CanTp.h"
#include "Dcm/Dcm.h"
#include "Log.h"
#include <string.h>

/* ========================================================================= */
/* BIẾN TRẠNG THÁI VÀ BUFFER (RX)                                            */
/* ========================================================================= */
static CanTp_RxStateType    s_RxState = CANTP_RX_IDLE;
static uint8_t              s_RxBuffer[CANTP_MAX_PAYLOAD_SIZE];
static uint16_t             s_RxExpectedLen = 0U;
static uint16_t             s_RxIndex = 0U;
static uint8_t              s_RxNextSn = 1U;
static uint32_t             s_RxTimerMs = 0U;

/* ========================================================================= */
/* BIẾN TRẠNG THÁI VÀ BUFFER (TX)                                            */
/* ========================================================================= */
static CanTp_TxStateType    s_TxState = CANTP_TX_IDLE;
static uint8_t              s_TxBuffer[CANTP_MAX_PAYLOAD_SIZE];
static uint16_t             s_TxTotalLen = 0U;
static uint16_t             s_TxIndex = 0U;
static uint8_t              s_TxNextSn = 1U;
static uint32_t             s_TxTimerMs = 0U;
static uint8_t              s_TxStMin = CANTP_STMIN_MS; /* Giãn cách nhả CF */
static uint8_t              s_TxBlockSize = 0U;         /* 0 = Bắn CF liên tục không cần chờ FC nữa */
static uint8_t              s_TxCfCounter = 0U;

/* ========================================================================= */
/* HÀM NỘI BỘ                                                                */
/* ========================================================================= */

/** @brief Phát khung Flow Control (Clear to Send) khi nhận được First Frame */
static void CanTp_SendFlowControl(void)
{
    /* FC: [0x30] [BS=0] [STmin=0] [Padding] */
    uint8_t fcFrame[8] = {0x30U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U};
    CanIf_PduInfoType pdu = {
        .SduDataPtr = fcFrame,
        .SduLength = 8U
    };
    (void)Eal_CanIf_Transmit(CANIF_TX_PDU_UDS_RESP, &pdu);
}

/** @brief Băm payload và phát Consecutive Frame tiếp theo */
static void CanTp_SendNextConsecutiveFrame(void)
{
    uint8_t cfFrame[8];
    memset(cfFrame, 0x00, sizeof(cfFrame));

    /* PCI CF = 0x20 | SequenceNumber */
    cfFrame[0] = 0x20U | (s_TxNextSn & 0x0FU);
    s_TxNextSn = (s_TxNextSn + 1U) & 0x0FU;

    uint16_t remaining = s_TxTotalLen - s_TxIndex;
    uint8_t chunkSize = (remaining > 7U) ? 7U : (uint8_t)remaining;

    memcpy(&cfFrame[1], &s_TxBuffer[s_TxIndex], chunkSize);
    s_TxIndex += chunkSize;

    CanIf_PduInfoType pdu = {
        .SduDataPtr = cfFrame,
        .SduLength = 8U
    };
    (void)Eal_CanIf_Transmit(CANIF_TX_PDU_UDS_RESP, &pdu);

    if (s_TxIndex >= s_TxTotalLen) {
        /* Đã truyền xong toàn bộ */
        s_TxState = CANTP_TX_IDLE;
        LOG_I("CanTp TX: Multi-frame Transmit Complete.");
    } else {
        s_TxCfCounter++;
        if ((s_TxBlockSize > 0U) && (s_TxCfCounter >= s_TxBlockSize)) {
            /* Đã đạt giới hạn Block Size, phải dừng lại chờ Tester gửi FC tiếp */
            s_TxState = CANTP_TX_WAIT_FC;
            s_TxTimerMs = CANTP_N_BS_TIMEOUT_MS;
        } else {
            /* Vẫn còn trong Block, lên lịch bắn CF tiếp theo sau khoảng STmin */
            s_TxState = CANTP_TX_SEND_CF;
            s_TxTimerMs = s_TxStMin;
        }
    }
}

/* ========================================================================= */
/* API CÔNG KHAI                                                             */
/* ========================================================================= */

void CanTp_Init(void)
{
    s_RxState = CANTP_RX_IDLE;
    s_TxState = CANTP_TX_IDLE;
}

void CanTp_MainFunction(uint32_t elapsedMs)
{
    /* --- Xử lý Timer phía Nhận (N_Cr) --- */
    if (s_RxState == CANTP_RX_WAIT_CF) {
        if (s_RxTimerMs <= elapsedMs) {
            s_RxState = CANTP_RX_IDLE;
            LOG_E("CanTp RX: N_Cr Timeout! Dropped Multi-frame.");
        } else {
            s_RxTimerMs -= elapsedMs;
        }
    }

    /* --- Xử lý Timer phía Truyền (N_Bs & STmin) --- */
    if (s_TxState == CANTP_TX_WAIT_FC) {
        if (s_TxTimerMs <= elapsedMs) {
            s_TxState = CANTP_TX_IDLE;
            LOG_E("CanTp TX: N_Bs Timeout! Tester didn't send FC.");
        } else {
            s_TxTimerMs -= elapsedMs;
        }
    } else if (s_TxState == CANTP_TX_SEND_CF) {
        if (s_TxTimerMs <= elapsedMs) {
            s_TxTimerMs = 0U; /* Tắt timer */
            CanTp_SendNextConsecutiveFrame();
        } else {
            s_TxTimerMs -= elapsedMs;
        }
    }
}

void CanTp_RxIndication(CanIf_RxPduIdType RxPduId, const CanIf_PduInfoType *PduInfoPtr)
{
    (void)RxPduId;
    if ((PduInfoPtr == NULL) || (PduInfoPtr->SduDataPtr == NULL) || (PduInfoPtr->SduLength == 0U)) {
        return;
    }

    const uint8_t *pData = PduInfoPtr->SduDataPtr;
    uint8_t pciType = (pData[0] >> 4U) & 0x0FU;

    switch (pciType) {
        case 0x0U: {   /* --- SINGLE FRAME (SF) --- */
            uint8_t sfLen = pData[0] & 0x0FU;
            if ((sfLen > 0U) && (sfLen <= 7U)) {
                /* Xóa cỗ máy RX nếu đang dở dang */
                s_RxState = CANTP_RX_IDLE;
                /* Đẩy buffer thuần cho Dcm */
                Dcm_ProvideRxPayload(&pData[1], sfLen);
            }
            break;
        }

        case 0x1U: {    /* --- FIRST FRAME (FF) --- */ 
            s_RxExpectedLen = ((uint16_t)(pData[0] & 0x0FU) << 8U) | (uint16_t)pData[1];
            if ((s_RxExpectedLen >= 7U) && (s_RxExpectedLen <= CANTP_MAX_PAYLOAD_SIZE)) {
                memcpy(&s_RxBuffer[0], &pData[2], 6U);
                s_RxIndex = 6U;
                s_RxNextSn = 1U;
                s_RxState = CANTP_RX_WAIT_CF;
                s_RxTimerMs = CANTP_N_CR_TIMEOUT_MS;

                LOG_I("CanTp RX: Got First Frame. Expecting %u bytes.", s_RxExpectedLen);
                /* Phản hồi ngay Flow Control để Tester bơm CF */
                CanTp_SendFlowControl();
            } else {
                LOG_E("CanTp RX: FF Length %u out of bounds!", s_RxExpectedLen);
            }
            break;
        }
        
        case 0x2U: {    /* --- CONSECUTIVE FRAME (CF) --- */
            if (s_RxState ==  CANTP_RX_WAIT_CF) {
                uint8_t sn = pData[0] & 0x0FU;
                if (sn == s_RxNextSn) {
                    s_RxNextSn = (s_RxNextSn + 1U) & 0x0FU;
                    uint16_t remain = s_RxExpectedLen - s_RxIndex;
                    uint8_t chunk = (remain > 7U) ? 7U : remain;

                    memcpy(&s_RxBuffer[s_RxIndex], &pData[1], chunk);
                    s_RxIndex += chunk;
                    s_RxTimerMs = CANTP_N_CR_TIMEOUT_MS; /* Reset lại timeout */

                    if (s_RxIndex >= s_RxExpectedLen) {
                        s_RxState = CANTP_RX_IDLE;
                        LOG_I("CanTp RX: Multi-frame received successfully.");
                        /* Cung cấp dữ liệu hoàn chỉnh cho Dcm */
                        Dcm_ProvideRxPayload(s_RxBuffer, s_RxExpectedLen);
                    }
                } else {
                    s_RxState = CANTP_RX_IDLE;
                    LOG_E("CanTp RX: Wrong Sequence Number! Exp %u, Got %u", s_RxNextSn, sn);
                }
            }
            break;
        }

        case 0x3U: {    /* --- FLOW CONTROL (FC) --- */
            if (s_TxState == CANTP_TX_WAIT_FC) {
                uint8_t fs = pData[0] & 0x0FU;
                if (fs == 0x00U) {  /* CTS - Clear To Send */
                    s_TxBlockSize = pData[1];
                    s_TxStMin = pData[2];

                    /* Bảo vệ STmin tối thiểu */
                    if (s_TxStMin < CANTP_STMIN_MS) {
                        s_TxStMin = CANTP_STMIN_MS;
                    }

                    s_TxCfCounter = 0U;
                    s_TxState = CANTP_TX_SEND_CF;
                    s_TxTimerMs = s_TxStMin; /* Hẹn giờ để nhả CF */
                    LOG_I("CanTp TX: Got FC(CTS), BS = %u, STmin = %u", s_TxBlockSize, s_TxStMin);
                } else if (fs == 0x01U) {   /* WT - Wait */
                    s_TxTimerMs = CANTP_N_BS_TIMEOUT_MS; /* Chờ thêm 1 cục BS timeout nữa */
                } else {    /* OVFLW hoặc lỗi */
                    s_TxState = CANTP_TX_IDLE;
                    LOG_E("CanTp TX: FC Abort/Overflow.");
                }
            }
            break;
        }

        default:
            break;
    }
}

bool CanTp_Transmit(const uint8_t *pData, uint16_t length)
{
    if ((pData == NULL) || (length == 0U) || (length > CANTP_MAX_PAYLOAD_SIZE)) {
        return false;
    }

    if (s_TxState != CANTP_TX_IDLE) {
        LOG_W("CanTp TX: Busy, dropping transmit request.");
        return false;
    }

    if (length <= 7U) {
        /* Gửi Single Frame lập tức */
        uint8_t sfFrame[8];
        memset(sfFrame, 0x00, sizeof(sfFrame));
        sfFrame[0] = (uint8_t)(length & 0x0FU);
        memcpy(&sfFrame[1], pData, length);

        CanIf_PduInfoType pdu = {
            .SduDataPtr = sfFrame,
            .SduLength = 8U
        };
        return (Eal_CanIf_Transmit(CANIF_TX_PDU_UDS_RESP, &pdu) == 0U);
    } else {
        /* Bắt đầu chuỗi Multi-frame: Copy dữ liệu vào đệm */
        memcpy(s_TxBuffer, pData, length);
        s_TxTotalLen = length;
        s_TxIndex = 6U;
        s_TxNextSn = 1U;

        /* Phát First Frame */
        uint8_t ffFrame[8];
        ffFrame[0] = 0x10U | (uint8_t)((length >> 8U) & 0x0FU);
        ffFrame[1] = (uint8_t)(length & 0xFFU);
        memcpy(&ffFrame[2], &pData[0], 6U);

        CanIf_PduInfoType pdu = {
            .SduDataPtr = ffFrame,
            .SduLength = 8U
        };

        if (Eal_CanIf_Transmit(CANIF_TX_PDU_UDS_RESP, &pdu) == 0U) {
            /* Khởi động máy trạng thái chờ FC */
            s_TxState = CANTP_TX_WAIT_FC;
            s_TxTimerMs = CANTP_N_BS_TIMEOUT_MS;
            LOG_I("CanTp TX: Sent FF. Length %u. Waiting FC...", length);
            return true;
        }
    }
    return false;
}
