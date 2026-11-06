/**
 * @file CanNm_Types.h
 * @brief Định nghĩa các trạng thái và tham số thời gian cho AUTOSAR CAN Network Management.
 */

#ifndef CANNM_TYPES_H
#define CANNM_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define CANNM_NODE_ID                   0x01U   /**< Node ID của Radar Core ECU trên mạng CAN */
#define CANNM_MSG_CYCLE_MS              1000U   /**< Chu kỳ phát gói tin NM Keep-Alive (1000ms) */
#define CANNM_REPEAT_MSG_TIME_MS        2000U   /**< Thời gian duy trì Repeat Message State */
#define CANNM_TIMEOUT_TIME_MS           3000U   /**< Timeout mạng NM trước khi chuyển sang Prepare Bus-Sleep */
#define CANNM_WAIT_BUS_SLEEP_TIME_MS    1500U   /**< Thời gian chờ trước khi rơi vào Bus-Sleep */

/** @brief Các trạng thái chuẩn của máy trạng thái AUTOSAR CanNm */
typedef enum {
    CANNM_STATE_BUS_SLEEP           = 0U,
    CANNM_STATE_PREPARE_BUS_SLEEP   = 1U,
    CANNM_STATE_REPEAT_MESSAGE      = 2U,
    CANNM_STATE_NORMAL_OPERATION    = 3U,
    CANNM_STATE_READY_SLEEP         = 4U
} CanNm_StateType;

#endif /* CANNM_TYPES_H */
