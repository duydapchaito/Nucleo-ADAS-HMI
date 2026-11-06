/**
 * @file    Dcm.c
 * @brief   Triển khai điểm nhập (Entry-point) cho DCM và đóng gói Single Frame UDS qua CanIf.
 */

#include "Dcm/Dcm.h"
#include "Dcm/Dcm_Dsl.h"
#include "Dcm/Dcm_Dsd.h"
#include "Dcm/Dcm_Dsp.h"
#include "CanTp/CanTp.h"
#include <string.h>

/* Cấp phát tĩnh ở vùng nhớ .bss (Tuyệt đối không đặt mảng 256B trên Stack của Task) */
static uint8_t s_UdsResBuf[CANTP_MAX_PAYLOAD_SIZE];

void Dcm_Init(void)
{
    Dcm_Dsl_Init();
    Dcm_Dsp_Init();
}

void Dcm_MainFunction(uint32_t elapsedMs)
{
    Dcm_Dsl_MainFunction(elapsedMs);
}

void Dcm_ProvideRxPayload(const uint8_t *pReqPayload, uint16_t length)
{
    if ((pReqPayload == NULL) && (length == 0U)) {
        return;
    }

    uint16_t udsResLen = 0U;
    memset(s_UdsResBuf, 0x00, sizeof(s_UdsResBuf));

    /* DSD lấy payload mộc, tìm hàm và xử lý. Response ghi vào udsResBuf */
    if (Dcm_Dsd_Dispatch(pReqPayload, length, s_UdsResBuf, &udsResLen, CANTP_MAX_PAYLOAD_SIZE)) {
        /* Dcm_Dsd_Dispatch trả về true tức là phải phản hồi (Không có cờ Suppress) */
        (void)CanTp_Transmit(s_UdsResBuf, udsResLen);
    }
}
