/**
 * @file    Dcm_Dsd.h
 * @brief   Diagnostic Service Dispatcher - Phân luồng SID và đóng gói bản tin phản hồi.
 */

#ifndef DCM_DSD_H
#define DCM_DSD_H

#include "Dcm_Types.h"

/**
 * @brief  Nhận mảng payload UDS thô, tra bảng dịch vụ, kiểm tra quyền hạn và gọi hàm DSP tương ứng.
 * @param  pReqBuf:    Mảng chứa dữ liệu UDS Request (Byte 0 là SID).
 * @param  reqLen:     Tổng số byte của Request.
 * @param  pResBuf:    Mảng chứa dữ liệu UDS Response đầu ra.
 * @param  pResLen:    Con trỏ trả về độ dài thực tế của Response.
 * @param  maxResLen:  Kích thước tối đa của bộ đệm pResBuf.
 * @retval true nếu cần gửi gói phản hồi ra bus CAN, false nếu bị chặn (Suppress Positive Response).
 */
bool Dcm_Dsd_Dispatch(const uint8_t *pReqBuf, uint16_t reqLen, uint8_t *pResBuf, uint16_t *pResLen, uint16_t maxResLen);

#endif /* DCM_DSD_H */
