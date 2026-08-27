#include "Rte_Radar.h"
#include "Eal_RadarHw.h"
#include <stdint.h>

// ==========================================
// SHADOW BUFFERS
// (Dùng static để không ai bên ngoài file có thể truy cập trực tiếp)
// ==========================================

static uint32_t Rte_RawTicks_Buffer[RTE_MAX_SENSORS] = {0};

// Khởi tạo mặc định 400cm (Khoảng cách Max an toàn tuyệt đối)
static uint16_t Rte_Distance_Buffer[RTE_MAX_SENSORS] = {400, 400, 400, 400};

// Khởi tạo mặc định số P (An toàn nhất khi Boot)
static Rte_GearType Rte_CurrentGear = RTE_GEAR_P;

// ==========================================
// THỰC THI CÁC HÀM GIAO TIẾP
// ==========================================

void Rte_Write_RawTicks(Rte_SensorId_t SensorId, uint32_t Ticks)
{
    if (SensorId < RTE_MAX_SENSORS) {
        Rte_RawTicks_Buffer[SensorId] = Ticks;
    }
}

uint32_t Rte_Read_RawTicks(Rte_SensorId_t SensorId)
{
    if (SensorId < RTE_MAX_SENSORS) {
        return Rte_RawTicks_Buffer[SensorId];
    }
    return 0;
}

void Rte_Write_Distance(Rte_SensorId_t SensorId, uint16_t DistanceCm)
{
    if (SensorId < RTE_MAX_SENSORS) {
        Rte_Distance_Buffer[SensorId] = DistanceCm;
    }
}

uint16_t Rte_Read_Distance(Rte_SensorId_t SensorId)
{
    if (SensorId < RTE_MAX_SENSORS) {
        return Rte_Distance_Buffer[SensorId];
    }
    return 400;
}

void Rte_Write_Gear(Rte_GearType Gear) 
{
    Rte_CurrentGear = Gear;
}

Rte_GearType Rte_Read_Gear(void) 
{
    return Rte_CurrentGear;
}

uint16_t Rte_Prm_GetMinSensorRange(void)
{
    return EAL_RADAR_MIN_RANGE_CM;
}

uint16_t Rte_Prm_GetMaxSensorRange(void)
{
    return EAL_RADAR_MAX_RANGE_CM;
}
