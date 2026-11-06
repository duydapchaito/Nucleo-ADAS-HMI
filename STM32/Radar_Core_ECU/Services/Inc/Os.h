/**
 * @file Os.h
 * @brief Services OS Header - Điều phối Task thời gian thực, đồng bộ hóa tài nguyên và quản lý Reset.
 */

#ifndef OS_H
#define OS_H

#include <stdint.h>
#include <stdbool.h>
#include "Log.h"

/* ==================================================================== */
/* CẤU HÌNH TÀI NGUYÊN HỆ ĐIỀU HÀNH (OS RESOURCE CONFIGURATION)         */
/* ==================================================================== */

#define OS_CAN_RX_QUEUE_LEN         32U     /**< Sức chứa hàng đợi CAN RX */

/* Kích thước Stack cho từng Task (Đơn vị: Words = 4 Bytes) */
#define OS_STACK_WORDS_CAN_RX       1024U   /**< 4096 Bytes - CanTp + CanNm + DCM + DEM */
#define OS_STACK_WORDS_SENSOR_25MS  768U    /**< 3072 Bytes - Kalman FPU + Timers */
#define OS_STACK_WORDS_SWC_50MS     512U    /**< 2048 Bytes - AppL Radar + Com Manager */
#define OS_STACK_WORDS_LOGGER       512U    /**< 2048 Bytes - BSW Logger Task */

/* Mức ưu tiên Task */
#define OS_PRIO_CAN_RX              (tskIDLE_PRIORITY + 3U)
#define OS_PRIO_SENSOR_25MS         (tskIDLE_PRIORITY + 2U)
#define OS_PRIO_SWC_50MS            (tskIDLE_PRIORITY + 1U)
#define OS_PRIO_LOGGER              (tskIDLE_PRIORITY + 0U)

/** @brief Cấu trúc gói tin CAN trung chuyển qua hàng đợi OS */
typedef struct {
    uint32_t    CanId;
    uint8_t     Dlc;
    uint8_t     Data[8];
} Os_CanRxMsgType;

/* Khởi tạo hệ thống và điều khiển MCU Reset */
void Os_Init(void);
void OS_PerformReset(void);
void Os_DelayMs(uint32_t ms);

/* Quản lý Mutex bảo vệ RTE và EEPROM (NvM) */
void Os_Rte_Lock(void);
void Os_Rte_Unlock(void);
void Os_Nvm_Lock(void);
void Os_Nvm_Unlock(void);

/* Giao tiếp hàng đợi CAN RX & Đồng bộ hóa I2C */
void Os_CanRx_PushFromISR(uint32_t canId, uint8_t dlc, const uint8_t *pData);
bool Os_I2c_WaitForTransfer(uint32_t TimeoutMs);
void Os_I2c_ClearSemaphore(void);
void Os_I2c_TransferDoneFromISR(void);
 
#endif /* OS_H */
