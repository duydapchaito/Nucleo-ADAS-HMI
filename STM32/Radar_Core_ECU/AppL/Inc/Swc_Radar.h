/**
 * @file Swc_Radar.h
 * @brief Application Software Component Header - Định nghĩa ngưỡng khoảng cách 
 *        cảnh báo vùng và nguyên mẫu hàm nghiệp vụ radar.
 */

#ifndef SWC_RADAR_H
#define SWC_RADAR_H

#include <stdint.h>

/* Ngưỡng khoảng cách cho 4 vùng màu cảnh báo trên HMI (Đơn vị: cm) */
#define RADAR_ZONE_RED_MAX_CM       30U     /* <= 30cm: Vùng Đỏ (Nguy hiểm / Sát vật cản) */
#define RADAR_ZONE_ORANGE_MAX_CM    60U     /* 31..60cm: Vùng Cam (Cảnh báo gần) */
#define RADAR_ZONE_YELLOW_MAX_CM    120U    /* 61..120cm: Vùng Vàng (Chú ý) */
                                            /* > 120cm: Vùng Xanh lá (An toàn) */

/* Chu kỳ chia nhịp Buzzer (Tính theo số lần gọi hàm định kỳ 50ms) */
#define RADAR_BEEP_TICKS_ORANGE     2U      /* 2 * 50ms = 100ms đảo trạng thái còi */
#define RADAR_BEEP_TICKS_YELLOW     5U      /* 5 * 50ms = 250ms đảo trạng thái còi */

/** @brief Các mức cảnh báo ứng với màu hiển thị trên màn hình HMI Qt */
typedef enum {
    RADAR_WARN_GREEN = 0,   /* Xanh lá: An toàn     - Tắt còi */
    RADAR_WARN_YELLOW,      /* Vàng:    Chú ý       - Bíp chậm (250ms) */
    RADAR_WARN_ORANGE,      /* Cam:     Gần         - Bíp nhanh (100ms) */
    RADAR_WARN_RED          /* Đỏ:      Nguy hiểm   - Kêu liên hồi liên tục */
} Swc_Radar_WarnLevelType;

/**
 * @brief Khởi tạo module ứng dụng Swc_Radar.
 */
void Swc_Radar_Init(void);

/**
 * @brief Hàm thực thi chính chu kỳ 50ms của ứng dụng Radar.
 */
void Swc_Radar_MainFunction_50ms(void);

#endif /* SWC_RADAR_H */