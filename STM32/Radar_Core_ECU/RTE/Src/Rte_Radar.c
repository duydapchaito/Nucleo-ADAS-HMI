/**
 * @file Rte_Radar.c
 * @brief RTE Implementation - Cung cấp vùng nhớ đệm trung gian (Shadow Buffers) 
 *        và cơ chế bảo vệ đa nhiệm (Mutex Lock) chuẩn AUTOSAR giữa BSW và AppL.
 */

#include "Rte_Radar.h"
#include "Eal_RadarHw.h"
#include "Eal_Encoder.h"
#include "Eal_Buzzer.h"
#include "Cdd_Fusion.h"
#include "Com_Manager.h"
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

// Khởi tạo mặc định mức cảnh báo 0 (An toàn nhất)
static uint8_t Rte_CurrentWarnLevel = 0U;

// Khởi tạo mặc định index cảm biến vật lý
static uint8_t s_CurrentHwSensorIdx = 0U;

static const char s_GearCharMap[4] = {'P', 'R', 'N', 'D'};

void Rte_Radar_Init(void)
{
    Math_Fusion_Init(Rte_Prm_GetMinSensorRange(), Rte_Prm_GetMaxSensorRange());
    Eal_Radar_Init();
    Eal_Encoder_Init();
    Eal_Buzzer_TurnOff();

    s_CurrentHwSensorIdx = 0U;
    Eal_Radar_Trigger(s_CurrentHwSensorIdx);
}

void Rte_Bsw_MainFunction_25ms(void)
{
    uint32_t raw_ticks_4[RTE_MAX_HW_SENSORS];
    uint16_t unified_dist_6[RTE_MAX_SENSORS];
    uint8_t sensor_err_mask_6 = 0U;

    /* 1. Thu thập Ticks của mắt cảm biến vừa quét xong và lưu vào RTE */
    uint32_t ticks = Eal_Radar_GetRawTicks(s_CurrentHwSensorIdx);
    Rte_Write_RawTicks((Rte_HwSensorId_t)s_CurrentHwSensorIdx, ticks);

    /* 2. Chạy thuật toán lọc Kalman & Heuristic Fusion cho 6 mắt cảm biến */
    Rte_Read_AllRawTicks(raw_ticks_4);
    Math_Fusion_ProcessRadarArray(raw_ticks_4, 
                                Rte_Prm_GetMinSensorRange(), 
                                Rte_Prm_GetMaxSensorRange(),
                                unified_dist_6, 
                                &sensor_err_mask_6);
    
    Rte_Write_AllDistancesAndErr(unified_dist_6, sensor_err_mask_6);

    /* 3. Quét Encoder cần số và cập nhật trạng thái (P, R, N, D) */
    int32_t enc_delta = Eal_Encoder_GetDelta();
    if (enc_delta != 0) {
        int32_t gear = (int32_t)Rte_Read_Gear() + enc_delta;
        if (gear < (int32_t)RTE_GEAR_P) gear = (int32_t)RTE_GEAR_P;
        if (gear > (int32_t)RTE_GEAR_D) gear = (int32_t)RTE_GEAR_D;
        Rte_Write_Gear((Rte_GearType)gear);
    }

    /* 4. Kích hoạt xung Trigger cho mắt cảm biến kế tiếp (Time-Division Multiplexing) */
    s_CurrentHwSensorIdx = (s_CurrentHwSensorIdx + 1U) % (uint8_t)RTE_MAX_HW_SENSORS;
    Eal_Radar_Trigger(s_CurrentHwSensorIdx);
}

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
    uint32_t val = 0U;
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

void Rte_Write_WarnLevel(uint8_t WarnLevel)
{
    Os_Rte_Lock();
    Rte_CurrentWarnLevel = WarnLevel;
    Os_Rte_Unlock();
}

uint8_t Rte_Read_WarnLevel(void)
{
    uint8_t lvl;
    Os_Rte_Lock();
    lvl = Rte_CurrentWarnLevel;
    Os_Rte_Unlock();
    return lvl;
}

void Rte_Call_Buzzer_Actuate(Rte_BuzzerCmdType Cmd)
{
    switch (Cmd) {
        case RTE_BUZZER_ON:
            Eal_Buzzer_TurnOn();
            break;
        case RTE_BUZZER_TOGGLE:
            Eal_Buzzer_Toggle();
            break;
        case RTE_BUZZER_OFF:
        default:
            Eal_Buzzer_TurnOff();
            break;
    }
}

void Rte_Send_RadarComStatus(Rte_GearType Gear, const uint16_t Dist6[RTE_MAX_SENSORS], uint8_t ErrorMask)
{
    (void)Com_Manager_SendRadarCan((uint8_t)Gear, Dist6, ErrorMask);
    (void)Com_Manager_SendRadarNmea(s_GearCharMap[((uint8_t)Gear) & 0x03U], Dist6);
}

uint16_t Rte_Prm_GetMinSensorRange(void)
{
    return EAL_RADAR_MIN_RANGE_CM;
}

uint16_t Rte_Prm_GetMaxSensorRange(void)
{
    return EAL_RADAR_MAX_RANGE_CM;
}
