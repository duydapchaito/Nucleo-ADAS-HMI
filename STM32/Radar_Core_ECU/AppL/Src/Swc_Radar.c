/**
 * @file Swc_Radar.c
 * @brief Application Software Component - Xử lý logic nghiệp vụ 4 vùng cảnh báo,
 *        điều khiển nhịp còi Buzzer và quản lý tính hợp lý cần số (Gear Plausibility).
 */

#include "Swc_Radar.h"
#include "Rte_Radar.h"
#include "Com_Manager.h"
#include "Eal_Buzzer.h"
#include "Os.h"

static const char GearCharMap[4] = {'P', 'R', 'N', 'D'};

/* Nhóm 2-bit cao (Bit 7..6) của Byte 7 dùng cho chẩn đoán trạng thái cần số */
#define ERR_GEAR_SHIFT          6U
#define ERR_GEAR_OK             (0U << ERR_GEAR_SHIFT)
#define ERR_GEAR_OUT_OF_RANGE   (1U << ERR_GEAR_SHIFT)
#define ERR_GEAR_GLITCH         (2U << ERR_GEAR_SHIFT)
#define ERR_GEAR_LOST           (3U << ERR_GEAR_SHIFT)

/**
 * @brief Hàm phụ 1: Phân loại khoảng cách ngắn nhất vào 4 vùng cảnh báo (Xanh - Vàng - Cam - Đỏ).
 */
static Swc_Radar_WarnLevelType Swc_Radar_EvaluateWarnLevel(uint16_t min_dist)
{
    if (min_dist <= RADAR_ZONE_RED_MAX_CM) {
        return RADAR_WARN_RED;
    } else if (min_dist <= RADAR_ZONE_ORANGE_MAX_CM) {
        return RADAR_WARN_ORANGE;
    } else if (min_dist <= RADAR_ZONE_YELLOW_MAX_CM) {
        return RADAR_WARN_YELLOW;
    } else {
        return RADAR_WARN_GREEN;
    }
}

/**
 * @brief Hàm phụ 2: Điều khiển nhịp bíp còi Buzzer theo 4 vùng màu và trạng thái cần số.
 */
static void Swc_Radar_UpdateBuzzer4Levels(Rte_GearType gear, Swc_Radar_WarnLevelType warn_level)
{
    static uint8_t beep_tick_cnt = 0U;

    // Ở số P hoặc N: Luôn tắt còi hoàn toàn và reset bộ đếm nhịp
    if (gear != RTE_GEAR_D && gear != RTE_GEAR_R) {
        Eal_Buzzer_TurnOff();
        beep_tick_cnt = 0U;
        return;
    }

    switch (warn_level) {
        case RADAR_WARN_RED:
            /* Vùng Đỏ (<= 30cm): Kêu liên tục không ngắt quãng */
            Eal_Buzzer_TurnOn();
            beep_tick_cnt = 0U;
            break;

        case RADAR_WARN_ORANGE:
            /* Vùng Cam (31..60cm): Bíp nhanh mỗi 100ms (2 chu kỳ Task 50ms) */
            beep_tick_cnt++;
            if (beep_tick_cnt >= RADAR_BEEP_TICKS_ORANGE) {
                Eal_Buzzer_Toggle();
                beep_tick_cnt = 0U;
            }
            break;

        case RADAR_WARN_YELLOW:
            /* Vùng Vàng (61..120cm): Bíp chậm mỗi 250ms (5 chu kỳ Task 50ms) */
            beep_tick_cnt++;
            if (beep_tick_cnt >= RADAR_BEEP_TICKS_YELLOW) {
                Eal_Buzzer_Toggle();
                beep_tick_cnt = 0U;
            }
            break;
            
        case RADAR_WARN_GREEN:
        default:
            /* Vùng Xanh lá (> 120cm): An toàn, tắt còi */
            Eal_Buzzer_TurnOff();
            beep_tick_cnt = 0U;
            break;
    }
}

void Swc_Radar_Init(void)
{
    Eal_Buzzer_TurnOff();
    OS_LOG_I("Swc_Radar Initialized [Kalman + Heuristic Fusion Active].");
}

/**
 * @brief Hàm thực thi chính định kỳ 50ms của Software Component Radar.
 */
void Swc_Radar_MainFunction_50ms()
{
    static Rte_GearType prev_gear = RTE_GEAR_P;
    uint16_t dist_cm[RTE_MAX_SENSORS];
    uint8_t error_mask = 0x00U;
    uint16_t min_dist = Rte_Prm_GetMaxSensorRange();

    /* 1. Đọc mảng 6 cảm biến đồng đẳng và byte lỗi từ RTE */
    Rte_Read_AllDistancesAndErr(dist_cm, &error_mask);

    for (uint8_t i = 0; i < (uint8_t)RTE_MAX_SENSORS; i++)
    {
        if (dist_cm[i] < min_dist) {
            min_dist = dist_cm[i];
        }
    }

    /* 2. Đọc và chẩn đoán tính hợp lý của Cần số (Gear Plausibility Check) */
    Rte_GearType gear = Rte_Read_Gear();
    if (gear > RTE_GEAR_D) {
        error_mask |= ERR_GEAR_OUT_OF_RANGE;
        gear = RTE_GEAR_P;
    } else {
        int32_t stepp_diff = (int32_t) gear - (int32_t) prev_gear;
        if (stepp_diff > 1 || stepp_diff < -1) {
            error_mask |= ERR_GEAR_GLITCH;  // Phát hiện nhảy số bất thường
        } else {
            error_mask |= ERR_GEAR_OK;
        }
        prev_gear = gear;
    }

    /* 3. Đánh giá mức cảnh báo và cập nhật nhịp còi Buzzer */
    Swc_Radar_WarnLevelType warn_level = Swc_Radar_EvaluateWarnLevel(min_dist);
    Swc_Radar_UpdateBuzzer4Levels(gear, warn_level);

    /* 4. Truyền khung CAN 0x300 (8 Bytes) và chuỗi NMEA debug xuống tầng Com Manager */
    (void)Com_Manager_SendRadarCan((uint8_t)gear, dist_cm, error_mask);
    (void)Com_Manager_SendRadarNmea(GearCharMap[gear & 0x03U], dist_cm);
}
