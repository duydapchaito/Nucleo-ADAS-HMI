/**
 * @file CanTp.h
 * @brief Giao diện API của CanTp (Tầng Transport Protocol).
 */

#ifndef CANTP_H
#define CANTP_H

#include "CanTp_Types.h"
#include "Eal_CanIf.h"

void CanTp_Init(void);
void CanTp_MainFunction(uint32_t elapsedMs);

/* Tầng Eal_CanIf gọi lên khi nhận được ID 0x7E0 hoặc 0x7DF */
void CanTp_RxIndication(CanIf_RxPduIdType RxPduId, const CanIf_PduInfoType *PduInfoPtr);

/* Tầng Dcm gọi xuống để gửi phản hồi (Buffer thô, không quan tâm độ dài) */
bool CanTp_Transmit(const uint8_t *pData, uint16_t length);

#endif /* CANTP_H */
