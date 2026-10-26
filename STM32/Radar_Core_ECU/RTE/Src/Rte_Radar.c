/**
 * @file Rte_Radar.c
 * @brief RTE Implementation - Cung cấp vùng nhớ đệm trung gian (Shadow Buffers) 
 *        và cơ chế bảo vệ đa nhiệm (Mutex Lock) chuẩn AUTOSAR giữa BSW và AppL.
 */

#include "Rte_Radar.h"
#include "Eal_RadarHw.h"
#include "Os.h"
#include <string.h>

// ==========================================
// SHADOW BUFFERS (Vùng nhớ đệm chia sẻ an toàn)
// ==========================================
static uint32_t Rte_RawTicks_Buffer[RTE_MAX_HW_SENSORS] = {0};

// Khởi tạo mặc định khoảng cách Max an toàn tuyệt đối cho 6 mắt logic
static uint16_t Rte_Distance_Buffer[RTE_MAX_SENSORS] = {
    EAL_RADAR_MAX_RANGE_CM, EAL_RADAR_MAX_RANGE_CM, EAL_RADAR_MAX_RANGE_CM,
    EAL_RADAR_MAX_RANGE_CM, EAL_RADAR_MAX_RANGE_CM, EAL_RADAR_MAX_RANGE_CM
};

// Khởi tạo mặc định 0x00 (Không có lỗi cảm biến)
static uint8_t Rte_SensorErrMask_Buffer = 0x00U;

// Khởi tạo mặc định cần số ở vị trí P (An toàn nhất khi Boot)
static Rte_GearType Rte_CurrentGear = RTE_GEAR_P;

// ==========================================
// CÁC HÀM GIAO TIẾP RTE (API GETTER / SETTER)
// ==========================================

void Rte_Write_RawTicks(Rte_HwSensorId_t HwId, uint32_t Ticks)
{
    if (HwId < RTE_MAX_HW_SENSORS) {
        Os_Rte_Lock();
        Rte_RawTicks_Buffer[HwId] = Ticks;
        Os_Rte_Unlock();
    }
}

uint32_t Rte_Read_RawTicks(Rte_HwSensorId_t HwId)
{
    uint32_t val = 0;
    if (HwId < RTE_MAX_HW_SENSORS) {
        Os_Rte_Lock();
        val = Rte_RawTicks_Buffer[HwId];
        Os_Rte_Unlock();
    }
    return val;
}

void Rte_Read_AllRawTicks(uint32_t OutTicks4[RTE_MAX_HW_SENSORS])
{
    Os_Rte_Lock();
    memcpy(OutTicks4, Rte_RawTicks_Buffer, sizeof(uint32_t) * RTE_MAX_HW_SENSORS);
    Os_Rte_Unlock();
}

/**
 * @brief Ghi trọn bộ 6 khoảng cách đồng đẳng & byte lỗi vào RTE dưới một lần khóa Mutex duy nhất.
 */
void Rte_Write_AllDistancesAndErr(const uint16_t Dist6[RTE_MAX_SENSORS], uint8_t SensorErrMask6)
{
    Os_Rte_Lock();
    memcpy(Rte_Distance_Buffer, Dist6, sizeof(uint16_t) * RTE_MAX_SENSORS);
    Rte_SensorErrMask_Buffer = SensorErrMask6;
    Os_Rte_Unlock();
}

/**
 * @brief Swc_Radar múc trọn bộ 6 cảm biến đồng đẳng & byte lỗi ra xử lý.
 */
void Rte_Read_AllDistancesAndErr(uint16_t OutDist6[RTE_MAX_SENSORS], uint8_t *pOutSensorErrMask6)
{
    Os_Rte_Lock();
    memcpy(OutDist6, Rte_Distance_Buffer, sizeof(uint16_t) * RTE_MAX_SENSORS);
    if (pOutSensorErrMask6 != NULL) {
        *pOutSensorErrMask6 = Rte_SensorErrMask_Buffer;
    }
    Os_Rte_Unlock();
}

uint16_t Rte_Read_Distance(Rte_SensorId_t SensorId)
{
    uint16_t val = EAL_RADAR_MAX_RANGE_CM;
    if (SensorId < RTE_MAX_SENSORS) {
        Os_Rte_Lock();
        val = Rte_Distance_Buffer[SensorId];
        Os_Rte_Unlock();
    }
    return val;
}

void Rte_Write_Gear(Rte_GearType Gear) 
{
    Os_Rte_Lock();
    Rte_CurrentGear = Gear;
    Os_Rte_Unlock();
}

Rte_GearType Rte_Read_Gear(void) 
{
    Rte_GearType gear;
    Os_Rte_Lock();
    gear = Rte_CurrentGear;
    Os_Rte_Unlock();
    return gear;
}

uint16_t Rte_Prm_GetMinSensorRange(void)
{
    return EAL_RADAR_MIN_RANGE_CM;
}

uint16_t Rte_Prm_GetMaxSensorRange(void)
{
    return EAL_RADAR_MAX_RANGE_CM;
}
