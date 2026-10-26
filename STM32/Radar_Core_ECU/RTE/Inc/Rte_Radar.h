/**
 * @file Rte_Radar.h
 * @brief RTE Header - Định nghĩa các cổng giao tiếp (Ports), định danh cảm biến 
 *        và thông số cấu hình chuẩn AUTOSAR RTE.
 */

#ifndef RTE_RADAR_H
#define RTE_RADAR_H

#include <stdint.h>

/** @brief Danh mục 4 cảm biến phần cứng vật lý (Dành cho BSW: EAL & OS) */
typedef enum {
    RTE_HW_SENSOR_FL = 0,       /**< Trước trái (Front-Left) */
    RTE_HW_SENSOR_FR = 1,       /**< Trước phải (Front-Right) */
    RTE_HW_SENSOR_RL = 2,       /**< Sau trái (Rear-Left) */
    RTE_HW_SENSOR_RR = 3,       /**< Sau phải (Rear-Right) */
    RTE_MAX_HW_SENSORS = 4
} Rte_HwSensorId_t;

/** @brief Danh mục 6 cảm biến logic đồng đẳng (Dành cho AppL: Swc_Radar & HMI) */
typedef enum {
    RTE_SENSOR_FL = 0,          /**< Trước trái */
    RTE_SENSOR_FC = 1,          /**< Trước giữa (Mắt ảo hợp thể) */
    RTE_SENSOR_FR = 2,          /**< Trước phải */
    RTE_SENSOR_RL = 3,          /**< Sau trái */
    RTE_SENSOR_RC = 4,          /**< Sau giữa (Mắt ảo hợp thể) */
    RTE_SENSOR_RR = 5,          /**< Sau phải */
    RTE_MAX_SENSORS = 6,
} Rte_SensorId_t;

/** @brief Trạng thái vị trí cần số xe (Gear State) */
typedef enum {
    RTE_GEAR_P = 0,             /**< Số P (Park) */
    RTE_GEAR_R = 1,             /**< Số R (Reverse) */
    RTE_GEAR_N = 2,             /**< Số N (Neutral) */
    RTE_GEAR_D = 3              /**< Số D (Drive) */
} Rte_GearType;

// ==========================================
// CÁC HÀM API GETTER / SETTER
// ==========================================

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

/* Calibration Parameters */
uint16_t Rte_Prm_GetMinSensorRange(void);
uint16_t Rte_Prm_GetMaxSensorRange(void);

#endif /* RTE_RADAR_H */
