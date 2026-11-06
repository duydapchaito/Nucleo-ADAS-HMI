/**
 * @file    Dcm_Dsd.c
 * @brief   Triển khai bảng định tuyến dịch vụ (Service Table) và tạo khung phản hồi UDS.
 */

#include "Dcm/Dcm_Dsd.h"
#include "Dcm/Dcm_Dsl.h"
#include "Dcm/Dcm_Dsp.h"
#include "Log.h"

typedef Dcm_NrcType (*Dcm_ServiceHandlerType)(Dcm_MsgContextType *pMsgCtx);

typedef struct {
    uint8_t                 sid;
    bool                    allowInDefaultSession;
    Dcm_SecLevelType        minSecLevel;
    Dcm_ServiceHandlerType  handler;
} Dcm_ServiceConfigType;

/* Bảng cấu hình dịch vụ UDS của ECU */
static const Dcm_ServiceConfigType s_ServiceTable[] = {
    /* SID                      Cho phép ở Default?     Security tối thiểu      Hàm xử lý ở DSP */
    { DCM_SID_SESSION_CONTROL,  true,                   DCM_SEC_LEV_LOCKED,     Dcm_Dsp_SessionControl          },
    { DCM_SID_ECU_RESET,        true,                   DCM_SEC_LEV_LOCKED,     Dcm_Dsp_EcuReset                },
    { DCM_SID_TESTER_PRESENT,   true,                   DCM_SEC_LEV_LOCKED,     Dcm_Dsp_TesterPresent           },
    { DCM_SID_READ_DID,         true,                   DCM_SEC_LEV_LOCKED,     Dcm_Dsp_ReadDataByIdentifier    },
    { DCM_SID_WRITE_DID,        true,                   DCM_SEC_LEV_LOCKED,     Dcm_Dsp_WriteDataByIdentifier   }
};

#define DCM_NUM_SERVICES (sizeof(s_ServiceTable) / sizeof(s_ServiceTable[0]))

bool Dcm_Dsd_Dispatch(const uint8_t *pReqBuf, uint16_t reqLen, uint8_t *pResBuf, uint16_t *pResLen, uint16_t maxResLen)
{
    if ((pReqBuf == NULL) || (pResBuf == NULL) || (pResLen == NULL) || (reqLen == 0U) || (maxResLen < 3U)) {
        return false;
    }

    uint8_t sid = pReqBuf[0];
    const Dcm_ServiceConfigType *pFoundSvc = NULL;

    /* 1. Tìm kiếm SID trong bảng cấu hình */
    for (uint8_t i = 0U; i < DCM_NUM_SERVICES; i++) {
        if (s_ServiceTable[i].sid == sid) {
            pFoundSvc = &s_ServiceTable[i];
            break;
        }
    }

    Dcm_NrcType nrc = DCM_E_POSITIVE_RESPONSE;

    /* 2. Kiểm tra sự tồn tại của SID, quyền Session và quyền Security */
    if (pFoundSvc == NULL) {
        nrc = DCM_E_SERVICE_NOT_SUPPORTED;
    } else if ((!pFoundSvc->allowInDefaultSession) && (Dcm_Dsl_GetSession() == DCM_SESSION_DEFAULT)) {
        nrc = DCM_E_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION;
    } else if ((uint8_t)Dcm_Dsl_GetSecurityLevel() < (uint8_t)pFoundSvc->minSecLevel) {
        nrc = DCM_E_SECURITY_ACCESS_DENIED;
    } else {
        /* 3. Đóng gói ngữ cảnh và chuyển giao cho DSP thực thi */
        Dcm_MsgContextType msgCtx;
        msgCtx.reqData          = &pReqBuf[1];
        msgCtx.reqDataLen       = reqLen - 1U;
        msgCtx.resData          = &pResBuf[1];
        msgCtx.resDataLen       = 0U;
        msgCtx.resMaxLen        = maxResLen - 1U;
        msgCtx.suppressPosRsp   = false;

        /* Làm mới lại S3Server Timer mỗi khi nhận được yêu cầu dịch vụ hợp lệ */
        Dcm_Dsl_ResetS3Timer();

        LOG_I("DSD: Dispatching SID 0x%02X...", sid);

        nrc = pFoundSvc->handler(&msgCtx);

        LOG_I("DSD: Handler Done. NRC=0x%02X, ResLen=%u", nrc, msgCtx.resDataLen);

        if (nrc == DCM_E_POSITIVE_RESPONSE) {
            if (msgCtx.suppressPosRsp) {
                /* Ẩn gói phản hồi dương nếu bit SuppressPosRspMsgIndicationBit được bật */
                *pResLen = 0U;
                return false;
            }
            pResBuf[0] = sid + DCM_POSITIVE_RESP_OFFSET;
            *pResLen = msgCtx.resDataLen + 1U;
            return true; 
        }
    }

    /* 4. Đóng gói bản tin Phản hồi âm (Negative Response: [0x7F] [SID] [NRC]) */
    pResBuf[0]  = DCM_NEGATIVE_RESP_SID;
    pResBuf[1]  = sid;
    pResBuf[2]  = (uint8_t)nrc;
    *pResLen    = 3U;
    return true;
}
