/**
 * @file Rte_Radar.h
 * @brief RTE Header - Định nghĩa cổng giao tiếp (Ports) chuẩn AUTOSAR RTE giữa AppL và BSW.
 */

#ifndef RTE_RADAR_H
#define RTE_RADAR_H

#include <stdint.h>

/** @brief Danh mục 4 cảm biến phần cứng vật lý (Dành cho BSW: EAL & OS) */
typedef enum {
    RTE_HW_SENSOR_FL    = 0U,   /**< Trước trái (Front-Left) */
    RTE_HW_SENSOR_FR    = 1U,   /**< Trước phải (Front-Right) */
    RTE_HW_SENSOR_RL    = 2U,   /**< Sau trái (Rear-Left) */
    RTE_HW_SENSOR_RR    = 3U,   /**< Sau phải (Rear-Right) */
    RTE_MAX_HW_SENSORS  = 4U
} Rte_HwSensorId_t;

/** @brief Danh mục 6 cảm biến logic đồng đẳng (Dành cho AppL: Swc_Radar & HMI) */
typedef enum {
    RTE_SENSOR_FL   = 0U,   /**< Trước trái */
    RTE_SENSOR_FC   = 1U,   /**< Trước giữa (Mắt ảo hợp thể) */
    RTE_SENSOR_FR   = 2U,   /**< Trước phải */
    RTE_SENSOR_RL   = 3U,   /**< Sau trái */
    RTE_SENSOR_RC   = 4U,   /**< Sau giữa (Mắt ảo hợp thể) */
    RTE_SENSOR_RR   = 5U,   /**< Sau phải */
    RTE_MAX_SENSORS = 6U
} Rte_SensorId_t;

/** @brief Trạng thái vị trí cần số xe (Gear State) */
typedef enum {
    RTE_GEAR_P = 0U,    /**< Số P (Park) */
    RTE_GEAR_R = 1U,    /**< Số R (Reverse) */
    RTE_GEAR_N = 2U,    /**< Số N (Neutral) */
    RTE_GEAR_D = 3U     /**< Số D (Drive) */
} Rte_GearType;

/** @brief Lệnh điều khiển còi Buzzer từ AppL xuống EAL qua RTE */
typedef enum {
    RTE_BUZZER_OFF      = 0U,
    RTE_BUZZER_ON       = 1U,
    RTE_BUZZER_TOGGLE   = 2U
} Rte_BuzzerCmdType;

/* ========================================== */
/* KHỞI TẠO VÀ CHU KỲ THU THẬP BSW QUA RTE    */
/* ========================================== */

void Rte_Radar_Init(void);
void Rte_Bsw_MainFunction_25ms(void);

/* ========================================== */
/* CÁC HÀM API SENDER-RECEIVER & CLIENT-SERVER*/
/* ========================================== */

/* BSW RawTicks Port (4 kênh vật lý) */
void Rte_Write_RawTicks(Rte_HwSensorId_t HwId, uint32_t Ticks);
uint32_t Rte_Read_RawTicks(Rte_HwSensorId_t HwId);
void Rte_Read_AllRawTicks(uint32_t OutTicks4[RTE_MAX_HW_SENSORS]);

/* Unified 6-Sensor Distance & ErrorMask Port (6 kênh đồng đẳng cho Swc_Radar) */
void Rte_Write_AllDistancesAndErr(const uint16_t Dist6[RTE_MAX_SENSORS], uint8_t SensorErrMask6);
void Rte_Read_AllDistancesAndErr(uint16_t OutDist6[RTE_MAX_SENSORS], uint8_t *pOutSensorErrMask6);
uint16_t Rte_Read_Distance(Rte_SensorId_t SensorId);

/* Gear Port */
void Rte_Write_Gear(Rte_GearType Gear);
Rte_GearType Rte_Read_Gear(void);

/* Warn Level */
void Rte_Write_WarnLevel(uint8_t WarnLevel);
uint8_t Rte_Read_WarnLevel(void);

/* Cổng Client-Server điều khiển Buzzer và phát bản tin COM (0x300 + NMEA) */
void Rte_Call_Buzzer_Actuate(Rte_BuzzerCmdType Cmd);
void Rte_Send_RadarComStatus(Rte_GearType Gear, const uint16_t Dist6[RTE_MAX_SENSORS], uint8_t ErrorMask);

/* Calibration Parameters */
uint16_t Rte_Prm_GetMinSensorRange(void);
uint16_t Rte_Prm_GetMaxSensorRange(void);

#endif /* RTE_RADAR_H */
