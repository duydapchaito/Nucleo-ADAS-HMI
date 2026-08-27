#ifndef LIB_FILTER_H
#define LIB_FILTER_H

#include <stdint.h>

#define MEDIAN_WINDOW_SIZE  5

// =========================================
// 1. CẤU TRÚC LỌC TRUNG VỊ (MEDIAN)
// =========================================
typedef struct {
    uint16_t Buffer[MEDIAN_WINDOW_SIZE];
    uint8_t Index;
    uint8_t Is_Filled;
} Math_Median_t;

// =========================================
// 2. CẤU TRÚC LỌC KALMAN 1 CHIỀU
// =========================================
typedef struct {
    float Q; // Nhiễu hệ thống (Process Noise)
    float R; // Nhiễu đo lường (Measurement Noise)
    float P; // Sai số ước lượng (Estimation Error Covariance)
    float K; // Hệ số Kalman (Kalman Gain)
    float X; // Giá trị ước lượng hiện tại (Estimated Value)
} Math_Kalman1D_t;

// =========================================
// CÁC HÀM API ĐỘC LẬP (PIPELINE)
// =========================================
// Tầng 1: Lọc thô (2 - 400cm)
uint16_t Math_Filter_Outlier(uint16_t raw_val, uint16_t min_limit, uint16_t max_limit, uint16_t last_valid_val);

// Tầng 2: Lọc trung vị (Khử nhiễu độc biến)
void Math_Filter_MedianInit(Math_Median_t *filter);
uint16_t Math_Filter_MedianProcess(Math_Median_t *filter, uint16_t new_val);

// Tầng 3: Lọc Kalman 1 chiều (Làm mượt, dự đoán)
void Math_Filter_KalmanInit(Math_Kalman1D_t *filter, float q, float r, float initial_p, float initial_x);
float Math_Filter_KalmanProcess(Math_Kalman1D_t* filter, float measurement);

#endif /* LIB_FILTER_H */
