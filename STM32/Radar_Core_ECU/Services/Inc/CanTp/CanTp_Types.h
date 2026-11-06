/**
 * @file CanTp_Types.h
 * @brief Định nghĩa các kiểu dữ liệu và cấu hình cho ISO 15765-2 CanTp.
 */

#ifndef CANTP_TYPES_H
#define CANTP_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* Cấu hình Buffer tối đa cho chẩn đoán (Đủ cho Read DID dài, hoặc DTC Snapshot) */
#define CANTP_MAX_PAYLOAD_SIZE      256U

/* Định mức Timeout theo chuẩn ISO 15765-2 */
#define CANTP_N_AR_TIMEOUT_MS       1000U   /* Timeout truyền khung vật lý */
#define CANTP_N_CR_TIMEOUT_MS       1000U   /* Timeout chờ CF tiếp theo */
#define CANTP_N_BS_TIMEOUT_MS       1000U   /* Timeout chờ FC từ Tester */
#define CANTP_STMIN_MS              10U     /* Khoảng giãn cách tối thiểu giữa 2 CF phát đi (Tx) */

/** @brief Máy trạng thái phía nhận (RX) */
typedef enum {
    CANTP_RX_IDLE       = 0U,
    CANTP_RX_WAIT_CF    = 1U
} CanTp_RxStateType;

/** @brief Máy trạng thái phía phát (TX) */
typedef enum {
    CANTP_TX_IDLE       = 0U,
    CANTP_TX_WAIT_FC    = 1U,
    CANTP_TX_SEND_CF    = 2U
} CanTp_TxStateType;

#endif /* CANTP_TYPES_H */
