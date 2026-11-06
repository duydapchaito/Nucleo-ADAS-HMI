/**
 * @file    Dcm_Dsl.h
 * @brief   Diagnostic Session Layer - Quản lý trạng thái Session, Security và S3Server Timer.
 */

#ifndef DCM_DSL_H
#define DCM_DSL_H

#include "Dcm_Types.h"

void                Dcm_Dsl_Init(void);
void                Dcm_Dsl_MainFunction(uint32_t elapsedMs);

void                Dcm_Dsl_SetSession(Dcm_SessionType newSession);
Dcm_SessionType     Dcm_Dsl_GetSession(void);

void                Dcm_Dsl_SetSecurityLevel(Dcm_SecLevelType newSecLevel);
Dcm_SecLevelType    Dcm_Dsl_GetSecurityLevel(void);

void                Dcm_Dsl_ResetS3Timer(void);

void                Dcm_Dsl_RequestReset(uint8_t resetType);

#endif /* DCM_DSL_H */
