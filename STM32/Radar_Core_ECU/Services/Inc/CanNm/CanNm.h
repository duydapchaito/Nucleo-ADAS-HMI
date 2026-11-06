/**
 * @file CanNm.h
 * @brief Giao diện API công khai của module CAN Network Management (CanNm).
 */

#ifndef CANNM_H
#define CANNM_H

#include "CanNm_Types.h"
#include "Eal_CanIf.h"

void            CanNm_Init(void);
void            CanNm_MainFunction(uint32_t elapsedMs);

/* Callback từ Eal_CanIf khi nhận được gói tin NM (0x500..0x5FF) trên bus CAN */
void            CanNm_RxIndication(CanIf_RxPduIdType RxPduId, const CanIf_PduInfoType *PduInfoPtr);

/* API điều khiển trạng thái mạng từ hệ thống (BswM / OS / Com) */
void            CanNm_NetworkRequest(void);
void            CanNm_NetworkRelease(void);
CanNm_StateType CanNm_GetState(void);

#endif /* CANNM_H */
