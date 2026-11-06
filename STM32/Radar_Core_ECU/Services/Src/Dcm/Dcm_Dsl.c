/**
 * @file    Dcm_Dsl.c
 * @brief   Triển khai quản lý phiên làm việc, S3Server Timer và uỷ quyền Reset qua OS.
 */

#include "Dcm/Dcm_Dsl.h"
#include "Os.h"
#include "Log.h"

static Dcm_SessionType  s_CurrentSession    = DCM_SESSION_DEFAULT;
static Dcm_SecLevelType s_CurrentSecLevel   = DCM_SEC_LEV_LOCKED;
static uint32_t         s_S3TimerMs         = 0U;
static uint8_t          s_PendingResetType  = 0U;
static uint32_t         s_ResetDelayMs      = 0U;

void Dcm_Dsl_Init(void)
{
    s_CurrentSession    = DCM_SESSION_DEFAULT;
    s_CurrentSecLevel   = DCM_SEC_LEV_LOCKED;
    s_S3TimerMs         = 0U;
    s_PendingResetType  = 0U;
    s_ResetDelayMs      = 0U;
}

void Dcm_Dsl_SetSession(Dcm_SessionType newSession)
{
    s_CurrentSession = newSession;

    if (newSession == DCM_SESSION_DEFAULT) {
        /* Khi quay về Default Session, tự động khóa lại quyền Security và tắt S3 Timer */
        s_CurrentSecLevel = DCM_SEC_LEV_LOCKED;
        s_S3TimerMs = 0U;
    } else {
        /* Kích hoạt bộ đếm S3Server cho các phiên mở rộng */
        s_S3TimerMs = DCM_S3_SERVER_TIMEOUT_MS;
    }
}

Dcm_SessionType Dcm_Dsl_GetSession(void)
{
    return s_CurrentSession;
}

void Dcm_Dsl_SetSecurityLevel(Dcm_SecLevelType newSecLevel)
{
    s_CurrentSecLevel = newSecLevel;
}

Dcm_SecLevelType Dcm_Dsl_GetSecurityLevel(void)
{
    return s_CurrentSecLevel;
}

void Dcm_Dsl_ResetS3Timer(void)
{
    if (s_CurrentSession != DCM_SESSION_DEFAULT) {
        s_S3TimerMs = DCM_S3_SERVER_TIMEOUT_MS;
    }
}

void Dcm_Dsl_RequestReset(uint8_t resetType)
{
    s_PendingResetType = resetType;
    s_ResetDelayMs = 50U; /* Đợi 50ms để CAN kịp phát xong gói 0x51 */
}

void Dcm_Dsl_MainFunction(uint32_t elapsedMs)
{
    if (s_PendingResetType != 0U) {
        if (s_ResetDelayMs <= elapsedMs) {
            s_PendingResetType = 0U;
            OS_PerformReset();
        } else {
            s_ResetDelayMs -= elapsedMs;
        }
    }

    if ((s_CurrentSession != DCM_SESSION_DEFAULT) && (s_S3TimerMs > 0U)) {
        if (s_S3TimerMs <= elapsedMs) {
            s_S3TimerMs         = 0U;
            s_CurrentSession    = DCM_SESSION_DEFAULT;
            s_CurrentSecLevel   = DCM_SEC_LEV_LOCKED;
            LOG_W("DCM: S3 Timeout -> Fallback to DEFAULT Session");
        } else {
            s_S3TimerMs -= elapsedMs;
        }
    }
}
