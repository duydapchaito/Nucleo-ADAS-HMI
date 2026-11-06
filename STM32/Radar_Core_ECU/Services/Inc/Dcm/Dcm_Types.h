/**
 * @file    Dcm_Types.h
 * @brief   Định nghĩa các kiểu dữ liệu, mã SID, DID (0x0100..0x0104) và NRC chuẩn ISO 14229.
 */

#ifndef DCM_TYPES_H
#define DCM_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ==================================================================== */
/* CẤU HÌNH THỜI GIAN CHUẨN CHẨN ĐOÁN                          */
/* ==================================================================== */
#define DCM_S3_SERVER_TIMEOUT_MS    5000U   /**< Thời gian tự động rớt về Default Session (5000ms) */
#define DCM_MAX_BUFFER_SIZE         64U     /**< Kích thước bộ đệm PDU chẩn đoán tối đa */

/* ==================================================================== */
/* MÃ DỊCH VỤ UDS (SERVICE IDENTIFIER - SID)                            */
/* ==================================================================== */
#define DCM_SID_SESSION_CONTROL     0x10U
#define DCM_SID_ECU_RESET           0x11U
#define DCM_SID_CLEAR_DTC           0x14U
#define DCM_SID_READ_DTC            0x19U
#define DCM_SID_READ_DID            0x22U
#define DCM_SID_SECURITY_ACCESS     0x27U
#define DCM_SID_WRITE_DID           0x2EU
#define DCM_SID_ROUTINE_CONTROL     0x31U
#define DCM_SID_TESTER_PRESENT      0x3EU

#define DCM_POSITIVE_RESP_OFFSET    0x40U   /**< Độ lệch cộng thêm vào SID khi phản hồi dương (Positive) */
#define DCM_NEGATIVE_RESP_SID       0x7FU   /**< Byte đầu tiên của bản tin phản hồi âm (Negative) */
#define DCM_SUPPRESS_POS_RSP_BIT    0x80U   /**< Bit 7 yêu cầu ẩn phản hồi dương ở các dịch vụ có SubFunction */

/* ==================================================================== */
/* DANH SÁCH DATA IDENTIFIER (DID) PHỤC VỤ TEST NVM                     */
/* ==================================================================== */
#define DCM_DID_DEFAULT_PARAM_TEST  0x0100U /**< 4B: Đọc/ghi tham số mặc định EEPROM AT24C02 */
#define DCM_DID_DEFAULT_PARAM_LEN   4U

#define DCM_DID_RADAR_HW_RAW        0x0101U /**< 5B: 4 mắt phần cứng thô (FL, FR, RL, RR) + Raw ErrMask */
#define DCM_DID_RADAR_LOGIC_DIST    0x0102U /**< 7B: 6 mắt cảm biến logic (FL..RR) + Logic ErrMask */
#define DCM_DID_RADAR_SYS_STATUS    0x0103U /**< 5B: Gear, WarnLevel, ErrorMask, Session, SecLevel */
#define DCM_DID_RADAR_FULL_TELEM    0x0104U /**< 11B: Gear, 6 mắt logic, Warn, Error, Session, SecLevel */

/* ==================================================================== */
/* CÁC PHIÊN CHẨN ĐOÁN (DIAGNOSTIC SESSIONS) & BẢO MẬT                  */
/* ==================================================================== */
typedef enum {
    DCM_SESSION_DEFAULT     = 0x01U,    /**< Phiên mặc định khi khởi động */
    DCM_SESSION_PROGRAMMING = 0x02U,    /**< Phiên nạp phần mềm */
    DCM_SESSION_EXTENDED    = 0x03U     /**< Phiên chẩn đoán mở rộng */
} Dcm_SessionType;

typedef enum {
    DCM_SEC_LEV_LOCKED      = 0x00U,    /**< Trạng thái khóa bảo mật */
    DCM_SEC_LEV_L1          = 0x01U,    /**< Cấp bảo mật 1 (Đọc/Xóa dữ liệu nhạy cảm) */
    DCM_SEC_LEV_L2          = 0x02U     /**< Cấp bảo mật 2 (Ghi cấu hình / Chạy Routine) */
} Dcm_SecLevelType;

/* ==================================================================== */
/* MÃ PHẢN HỒI ÂM CHUẨN ISO 14229 (NEGATIVE RESPONSE CODE - NRC)        */
/* ==================================================================== */
typedef enum {
    DCM_E_POSITIVE_RESPONSE                             = 0x00U,    /**< Thực thi thành công (Không lỗi) */
    DCM_E_GENERAL_REJECT                                = 0x10U,    /**< Từ chối chung */
    DCM_E_SERVICE_NOT_SUPPORTED                         = 0x11U,    /**< SID không được hỗ trợ */
    DCM_E_SUB_FUNCTION_NOT_SUPPORTED                    = 0x12U,    /**< Sub-function không hỗ trợ */
    DCM_E_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT    = 0x13U,    /**< Sai độ dài bản tin hoặc sai định dạng */
    DCM_E_CONDITIONS_NOT_CORRECTED                      = 0x22U,    /**< Điều kiện thực thi không thỏa mãn */
    DCM_E_REQUEST_OUT_OF_RANGE                          = 0x31U,    /**< DID hoặc tham số nằm ngoài dải cho phép */
    DCM_E_SECURITY_ACCESS_DENIED                        = 0x33U,    /**< Chưa mở khóa bảo mật (0x27) */
    DCM_E_INVALID_KEY                                   = 0x35U,    /**< Gửi sai mã Key bảo mật */
    DCM_E_EXCEED_NUMBER_OF_ATTEMPTS                     = 0x36U,    /**< Nhập sai Key quá số lần quy định */
    DCM_E_GENERAL_PROGRAMMING_FAILURE                   = 0x72U,    /**< Lỗi ghi phần cứng bộ nhớ (EEPROM/Flash) */
    DCM_E_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION       = 0x7FU     /**< SID không được phép chạy ở Session hiện tại */
} Dcm_NrcType;

/* ==================================================================== */
/* CẤU TRÚC NGỮ CẢNH BẢN TIN CHẨN ĐOÁN (MESSAGE CONTEXT)                */
/* ==================================================================== */
typedef struct {
    const uint8_t   *reqData;       /**< Con trỏ tới dữ liệu Request (bỏ qua byte SID đầu tiên) */
    uint16_t        reqDataLen;     /**< Độ dài phần dữ liệu Request (không tính byte SID) */
    uint8_t         *resData;       /**< Con trỏ tới vùng đệm chứa dữ liệu Response (bắt đầu sau byte SID+0x40) */
    uint16_t        resDataLen;     /**< Độ dài dữ liệu Response thực tế do DSP điền vào */
    uint16_t        resMaxLen;      /**< Kích thước tối đa của vùng đệm Response */
    bool            suppressPosRsp; /**< Cờ báo ẩn gói phản hồi dương (SuppressPosRspMsgIndicationBit) */
} Dcm_MsgContextType;

#endif /* DCM_TYPES_H */
