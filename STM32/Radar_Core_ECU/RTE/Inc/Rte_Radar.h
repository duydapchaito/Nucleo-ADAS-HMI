#ifndef RTE_RADAR_H
#define RTE_RADAR_H

#include <stdint.h>

// 1. Định nghĩa chuẩn hoá ID 4 góc (FL, FR, RL, RR)
typedef enum {
    RTE_SENSOR_FL = 0,
    RTE_SENSOR_FR,
    RTE_SENSOR_RL,
    RTE_SENSOR_RR,
    RTE_MAX_SENSORS
} Rte_SensorId_t;

// 2. Định nghĩa trạng thái Hộp số
typedef enum {
    RTE_GEAR_P = 0,
    RTE_GEAR_R,
    RTE_GEAR_N,
    RTE_GEAR_D
} Rte_GearType;

// ==========================================
// CÁC HÀM API GETTER / SETTER
// ==========================================

// Giao tiếp Ticks thô (Dành cho EAL -> RTE -> AppL)
void Rte_Write_RawTicks(Rte_SensorId_t SensorId, uint32_t Ticks);
uint32_t Rte_Read_RawTicks(Rte_SensorId_t SensorId);

// Giao tiếp Khoảng cách đã qua bộ lọc Kalman (AppL -> RTE -> Services)
void Rte_Write_Distance(Rte_SensorId_t SensorId, uint16_t DistanceCm);
uint16_t Rte_Read_Distance(Rte_SensorId_t SensorId);

// Giao tiếp Hộp số
void Rte_Write_Gear(Rte_GearType Gear);
Rte_GearType Rte_Read_Gear(void);

// API lấy thông số đặc tả phần cứng (Calibration / Configuration)
uint16_t Rte_Prm_GetMinSensorRange(void);
uint16_t Rte_Prm_GetMaxSensorRange(void);

#endif /* RTE_RADAR_H */
