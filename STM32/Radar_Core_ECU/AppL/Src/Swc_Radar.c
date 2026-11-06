/**
 * @file Swc_Radar.c
 * @brief Application Software Component - Xử lý logic nghiệp vụ 4 vùng cảnh báo,
 *        điều khiển nhịp còi Buzzer và kiểm tra tính hợp lý cần số hoàn toàn qua RTE.
 */

#include "Swc_Radar.h"
#include "Rte_Radar.h"

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
    if ((gear != RTE_GEAR_D) && (gear != RTE_GEAR_R)) {
        Rte_Call_Buzzer_Actuate(RTE_BUZZER_OFF);
        beep_tick_cnt = 0U;
        return;
    }

    switch (warn_level) {
        case RADAR_WARN_RED:
            /* Vùng Đỏ (<= 30cm): Kêu liên tục không ngắt quãng */
            Rte_Call_Buzzer_Actuate(RTE_BUZZER_ON);
            beep_tick_cnt = 0U;
            break;

        case RADAR_WARN_ORANGE:
            /* Vùng Cam (31..60cm): Bíp nhanh mỗi 100ms (2 chu kỳ Task 50ms) */
            beep_tick_cnt++;
            if (beep_tick_cnt >= RADAR_BEEP_TICKS_ORANGE) {
                Rte_Call_Buzzer_Actuate(RTE_BUZZER_TOGGLE);
                beep_tick_cnt = 0U;
            }
            break;

        case RADAR_WARN_YELLOW:
            /* Vùng Vàng (61..120cm): Bíp chậm mỗi 250ms (5 chu kỳ Task 50ms) */
            beep_tick_cnt++;
            if (beep_tick_cnt >= RADAR_BEEP_TICKS_YELLOW) {
                Rte_Call_Buzzer_Actuate(RTE_BUZZER_TOGGLE);
                beep_tick_cnt = 0U;
            }
            break;
            
        case RADAR_WARN_GREEN:
        default:
            /* Vùng Xanh lá (> 120cm): An toàn, tắt còi */
            Rte_Call_Buzzer_Actuate(RTE_BUZZER_OFF);
            beep_tick_cnt = 0U;
            break;
    }
}

void Swc_Radar_Init(void)
{
    Rte_Call_Buzzer_Actuate(RTE_BUZZER_OFF);
    Rte_Write_WarnLevel((uint8_t)RADAR_WARN_GREEN);
}

/**
 * @brief Hàm thực thi chính định kỳ 50ms của Software Component Radar.
 */
void Swc_Radar_MainFunction_50ms(void)
{
    static Rte_GearType prev_gear = RTE_GEAR_P;
    uint16_t dist_cm[RTE_MAX_SENSORS];
    uint8_t error_mask = 0x00U;
    uint16_t min_dist = Rte_Prm_GetMaxSensorRange();

    /* 1. Đọc mảng 6 cảm biến đồng đẳng và byte lỗi từ RTE */
    Rte_Read_AllDistancesAndErr(dist_cm, &error_mask);

    for (uint8_t i = 0U; i < (uint8_t)RTE_MAX_SENSORS; i++)
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
        int32_t step_diff = (int32_t) gear - (int32_t) prev_gear;
        if ((step_diff > 1) || (step_diff < -1)) {
            error_mask |= ERR_GEAR_GLITCH;  // Phát hiện nhảy số bất thường
        } else {
            error_mask |= ERR_GEAR_OK;
        }
        prev_gear = gear;
    }

    /* 3. Đánh giá mức cảnh báo và cập nhật nhịp còi Buzzer */
    Swc_Radar_WarnLevelType warn_level = Swc_Radar_EvaluateWarnLevel(min_dist);
    Rte_Write_WarnLevel((uint8_t)warn_level);
    Swc_Radar_UpdateBuzzer4Levels(gear, warn_level);

    /* 4. Phát gói tin CAN 0x300 (8 Bytes) và chuỗi NMEA qua cổng RTE */
    Rte_Send_RadarComStatus(gear, dist_cm, error_mask);
}
