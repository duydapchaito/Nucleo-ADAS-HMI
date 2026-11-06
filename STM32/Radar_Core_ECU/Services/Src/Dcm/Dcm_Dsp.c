/**
 * @file    Dcm_Dsp.c
 * @brief   Thực thi chi tiết các dịch vụ 0x10, 0x11, 0x3E, 0x22, 0x2E (Đọc/Ghi EEPROM & RTE).
 */

#include "Dcm/Dcm_Dsp.h"
#include "Dcm/Dcm_Dsl.h"
#include "Cdd_Eeprom.h"
#include "Rte_Radar.h"
#include "Log.h"

/* Giá trị khởi tạo mặc định (0xDEADBEEF) nếu EEPROM hoàn toàn trắng trong lần chạy đầu */
static const uint8_t s_DefaultParamInitVal[DCM_DID_DEFAULT_PARAM_LEN] = {
    0xDE, 0xAD, 0xBE, 0xEF
};

static inline uint8_t Dcm_Dsp_ScaleDistHalf(uint16_t dist_cm)
{
    uint16_t half = dist_cm >> 1U;
    return (half > 255U) ? 255U : (uint8_t)half;
}

void Dcm_Dsp_Init(void)
{
    uint8_t checkBuf[DCM_DID_DEFAULT_PARAM_LEN] = {0};

    /* Đọc kiểm tra ô nhớ Default Param lúc khởi động */
    if (Cdd_Eeprom_ReadBuffer(NVM_ADDR_DEFAULT_PARAM, checkBuf, DCM_DID_DEFAULT_PARAM_LEN) == CDD_EEPROM_OK) {
        /* Nếu cả 4 byte đều là 0xFF (chip EEPROM mới tinh chưa từng ghi), nạp giá trị mặc định vào */
        if ((checkBuf[0] == 0xFFU) && (checkBuf[1] == 0xFFU) &&
            (checkBuf[2] == 0xFFU) && (checkBuf[3] == 0xFFU)) {
            (void)Cdd_Eeprom_WriteBuffer(NVM_ADDR_DEFAULT_PARAM, s_DefaultParamInitVal, DCM_DID_DEFAULT_PARAM_LEN);
            LOG_I("DCM: Flashed Initial Default Param (DE AD BE EF) to EEPROM");
        } else {
            LOG_I("DCM: Loaded Default Param [%02X %02X %02X %02X]",
                    checkBuf[0], checkBuf[1], checkBuf[2], checkBuf[3]);
        }
    }
}

/**
 * @brief Dịch vụ 0x10: Diagnostic Session Control
 *        Format Request : [0x10] [SubFunction (1 Byte)]
 *        Format Response: [0x50] [SubFunction (1 Byte)]
 */
Dcm_NrcType Dcm_Dsp_SessionControl(Dcm_MsgContextType *pMsgCtx)
{
    if (pMsgCtx->reqDataLen != 1U) {
        return DCM_E_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT;
    }

    uint8_t rawSubFunc = pMsgCtx->reqData[0];
    uint8_t subFunc = rawSubFunc & 0x7FU;

    if ((subFunc == (uint8_t)DCM_SESSION_DEFAULT) ||
        (subFunc == (uint8_t)DCM_SESSION_PROGRAMMING) ||
        (subFunc == (uint8_t)DCM_SESSION_EXTENDED)) {
        Dcm_Dsl_SetSession((Dcm_SessionType)subFunc);
        pMsgCtx->suppressPosRsp = ((rawSubFunc & DCM_SUPPRESS_POS_RSP_BIT) != 0U);
        pMsgCtx->resData[0]     = subFunc;
        pMsgCtx->resDataLen     = 1U;
        LOG_I("DCM: Switched to Session 0x%02X", subFunc);
        return DCM_E_POSITIVE_RESPONSE;
    }

    return DCM_E_SUB_FUNCTION_NOT_SUPPORTED;
}

Dcm_NrcType Dcm_Dsp_EcuReset(Dcm_MsgContextType *pMsgCtx)
{
    if (pMsgCtx->reqDataLen != 1U) {
        return DCM_E_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT;
    }

    uint8_t rawSubFunc = pMsgCtx->reqData[0];
    uint8_t subFunc = rawSubFunc & 0x7FU;

    if ((subFunc == 0x01U) || (subFunc == 0x03U)) {
        pMsgCtx->suppressPosRsp = ((rawSubFunc & DCM_SUPPRESS_POS_RSP_BIT) != 0U);
        pMsgCtx->resData[0]     = subFunc;
        pMsgCtx->resDataLen     = 1U;
        LOG_W("DCM: ECU Reset (0x%02X) Requested!", subFunc);
        Dcm_Dsl_RequestReset(subFunc);
        return DCM_E_POSITIVE_RESPONSE;
    }

    return DCM_E_SUB_FUNCTION_NOT_SUPPORTED;
}

/**
 * @brief Dịch vụ 0x3E: Tester Present
 *        Format Request : [0x3E] [0x00]
 *        Format Response: [0x7E] [0x00]
 */
Dcm_NrcType Dcm_Dsp_TesterPresent(Dcm_MsgContextType *pMsgCtx)
{
    if (pMsgCtx->reqDataLen != 1U) {
        return DCM_E_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT;
    }

    uint8_t rawSubFunc = pMsgCtx->reqData[0];
    uint8_t subFunc = rawSubFunc & 0x7FU;

    if (subFunc != 0x00U) {
        return DCM_E_SUB_FUNCTION_NOT_SUPPORTED;
    }

    Dcm_Dsl_ResetS3Timer();
    pMsgCtx->suppressPosRsp = ((rawSubFunc & DCM_SUPPRESS_POS_RSP_BIT) != 0U);
    pMsgCtx->resData[0]     = 0x00U;
    pMsgCtx->resDataLen     = 1U;
    return DCM_E_POSITIVE_RESPONSE;
}

/**
 * @brief Dịch vụ 0x22: Read Data By Identifier (Đọc từ EEPROM AT24C02)
 *        Format Request : [0x22] [DID_High] [DID_Low]
 *        Format Response: [0x62] [DID_High] [DID_Low] [Data0 ... Data3]
 */
Dcm_NrcType Dcm_Dsp_ReadDataByIdentifier(Dcm_MsgContextType *pMsgCtx)
{
    if (pMsgCtx->reqDataLen != 2U) {
        return DCM_E_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT;
    }

    uint16_t did = ((uint16_t)pMsgCtx->reqData[0] << 8U) | ((uint16_t)pMsgCtx->reqData[1]);

    /* Điền lại 2 byte DID vào đầu vùng đệm phản hồi */
    pMsgCtx->resData[0] = pMsgCtx->reqData[0];
    pMsgCtx->resData[1] = pMsgCtx->reqData[1];

    if (did == DCM_DID_DEFAULT_PARAM_TEST) {
        /* DID 0x0100: 4 Bytes EEPROM Test */
        if (Cdd_Eeprom_ReadBuffer(NVM_ADDR_DEFAULT_PARAM, &pMsgCtx->resData[2], DCM_DID_DEFAULT_PARAM_LEN) == CDD_EEPROM_OK) {
            pMsgCtx->resDataLen = 2U + DCM_DID_DEFAULT_PARAM_LEN;
            LOG_I("DCM: Read DID 0x0100 OK -> [%02X %02X %02X %02X]",
                    pMsgCtx->resData[2], pMsgCtx->resData[3], pMsgCtx->resData[4], pMsgCtx->resData[5]);
            return DCM_E_POSITIVE_RESPONSE;
        }
        LOG_E("DCM: EEPROM Read Error DID 0x0100!");
        return DCM_E_GENERAL_PROGRAMMING_FAILURE;
    } else if (did == DCM_DID_RADAR_HW_RAW) {
        /* DID 0x0101 (5 Bytes Data): 4 mắt Raw (FL, FR, RL, RR scale 0.5x cm) + 1B Raw ErrMask */
        uint32_t ticks[RTE_MAX_HW_SENSORS];
        Rte_Read_AllRawTicks(ticks);

        uint8_t rawMask = 0U;
        for (uint8_t i = 0U; i < (uint8_t)RTE_MAX_HW_SENSORS; i++)
        {
            if (ticks[i] == 0U) {
                rawMask |= (uint8_t)(1U << i);
                pMsgCtx->resData[2U + i] = Dcm_Dsp_ScaleDistHalf((uint16_t)Rte_Prm_GetMaxSensorRange());
            } else {
                pMsgCtx->resData[2U + i] = Dcm_Dsp_ScaleDistHalf((uint16_t)(ticks[i] / 58U));
            }
        }
        pMsgCtx->resData[6] = rawMask;
        pMsgCtx->resDataLen = 2U + 5U; /* 2B DID + 5B Data = 7B (+1B SID 0x62 = 8B Multi-frame) */
        return DCM_E_POSITIVE_RESPONSE;
    } else if (did == DCM_DID_RADAR_LOGIC_DIST) {
        /* DID 0x0102 (7 Bytes Data): 6 cảm biến Logic (FL, FC, FR, RL, RC, RR) + 1B Logic ErrMask */
        uint16_t dist6[RTE_MAX_SENSORS];
        uint8_t logicErrMask = 0U;
        Rte_Read_AllDistancesAndErr(dist6, &logicErrMask);

        for (uint8_t i = 0U; i < (uint8_t)RTE_MAX_SENSORS; i++) 
        {
            pMsgCtx->resData[2U + i] = Dcm_Dsp_ScaleDistHalf(dist6[i]);
        }
        pMsgCtx->resData[8] = logicErrMask;
        pMsgCtx->resDataLen = 2U + 7U; /* 2B DID + 7B Data = 9B (+1B SID 0x62 = 10B Multi-frame) */
        return DCM_E_POSITIVE_RESPONSE;
    } else if (did == DCM_DID_RADAR_SYS_STATUS) {
        /* DID 0x0103 (5 Bytes Data): [Gear, Warn, ErrorMask, Session, SecLevel] */
        uint16_t dist6[RTE_MAX_SENSORS];
        uint8_t errMask = 0U;
        Rte_Read_AllDistancesAndErr(dist6, &errMask);

        pMsgCtx->resData[2] = (uint8_t)Rte_Read_Gear();
        pMsgCtx->resData[3] = Rte_Read_WarnLevel();
        pMsgCtx->resData[4] = errMask;
        pMsgCtx->resData[5] = (uint8_t)Dcm_Dsl_GetSession();
        pMsgCtx->resData[6] = (uint8_t)Dcm_Dsl_GetSecurityLevel();
        pMsgCtx->resDataLen = 2U + 5U; /* 2B DID + 5B Data = 7B (+1B SID 0x62 = 8B Multi-frame) */
        return DCM_E_POSITIVE_RESPONSE;
    } else if (did == DCM_DID_RADAR_FULL_TELEM) {
        /* DID 0x0104 (11 Bytes Data): [Gear, FL, FC, FR, RL, RC, RR, Warn, Error, Session, SecLevel] */
        uint16_t dist6[RTE_MAX_SENSORS];
        uint8_t errMask = 0U;
        Rte_Read_AllDistancesAndErr(dist6, &errMask);

        pMsgCtx->resData[2] = Rte_Read_Gear();
        for (uint8_t i = 0U; i < (uint8_t)RTE_MAX_SENSORS; i++) 
        {
            pMsgCtx->resData[3U + i] = Dcm_Dsp_ScaleDistHalf(dist6[i]);
        }

        pMsgCtx->resData[9] = Rte_Read_WarnLevel();
        pMsgCtx->resData[10] = errMask;
        pMsgCtx->resData[11] = (uint8_t)Dcm_Dsl_GetSession();
        pMsgCtx->resData[12] = (uint8_t)Dcm_Dsl_GetSecurityLevel();
        pMsgCtx->resDataLen = 13U; /* 2B DID + 11B Data = 13B (+1B SID 0x62 = 14B Multi-frame) */
        return DCM_E_POSITIVE_RESPONSE;
    }

    return DCM_E_REQUEST_OUT_OF_RANGE;
}

/**
 * @brief Dịch vụ 0x2E: Write Data By Identifier (Ghi xuống EEPROM AT24C02)
 *        Format Request : [0x2E] [DID_High] [DID_Low] [Data0] [Data1] [Data2] [Data3]
 *        Format Response: [0x6E] [DID_High] [DID_Low]
 */
Dcm_NrcType Dcm_Dsp_WriteDataByIdentifier(Dcm_MsgContextType *pMsgCtx)
{
    /* Kiểm tra độ dài ít nhất phải có 2 byte DID */
    if (pMsgCtx->reqDataLen < 2U) {
        return DCM_E_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT;
    }

    uint16_t did = ((uint16_t)pMsgCtx->reqData[0] << 8U) | (uint16_t)pMsgCtx->reqData[1];

    if (did == DCM_DID_DEFAULT_PARAM_TEST) {
        /* Với DID 0x0100, tổng chiều dài dữ liệu kèm theo bắt buộc là 2 (DID) + 4 (Data) = 6 bytes */
        if (pMsgCtx->reqDataLen != (2U + DCM_DID_DEFAULT_PARAM_LEN)) {
            return DCM_E_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT;
        }

        const uint8_t *pWriteBuffer = &pMsgCtx->reqData[2];

        /* Ghi 4 byte xuống địa chỉ 0x00 của AT24C02 */
        if (Cdd_Eeprom_WriteBuffer(NVM_ADDR_DEFAULT_PARAM, pWriteBuffer, DCM_DID_DEFAULT_PARAM_LEN) == CDD_EEPROM_OK) {
            pMsgCtx->resData[0] = pMsgCtx->reqData[0];
            pMsgCtx->resData[1] = pMsgCtx->reqData[1];
            pMsgCtx->resDataLen = 2U;
            LOG_I("DCM: Write DID 0x0100 OK <- [%02X %02X %02X %02X]",
                pWriteBuffer[0], pWriteBuffer[1], pWriteBuffer[2], pWriteBuffer[3]);
            return DCM_E_POSITIVE_RESPONSE;
        } else {
            LOG_E("DCM: EEPROM Write Error on DID 0x0100!");
            return DCM_E_GENERAL_PROGRAMMING_FAILURE;
        }
    }

    return DCM_E_REQUEST_OUT_OF_RANGE;
}
