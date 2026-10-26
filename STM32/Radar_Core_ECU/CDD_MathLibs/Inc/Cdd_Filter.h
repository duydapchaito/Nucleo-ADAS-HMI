/**
 * @file Cdd_Filter.h
 * @brief CDD Math Filter Header - Định nghĩa cấu trúc dữ liệu cho Median và Kalman 1D.
 */

#ifndef CDD_FILTER_H
#define CDD_FILTER_H

#include <stdint.h>

#define MEDIAN_WINDOW_SIZE  5U  /**< Kích thước cửa sổ trượt bộ lọc trung vị */

/** @brief Cấu trúc dữ liệu quản lý bộ lọc trung vị */
typedef struct {
    uint16_t Buffer[MEDIAN_WINDOW_SIZE];    /**< Mảng đệm chứa các mẫu giá trị */
    uint8_t Index;                          /**< Chỉ số con trỏ ghi mẫu hiện tại */
    uint8_t Is_Filled;                      /**< Cờ báo hiệu cửa sổ đã được lấp đầy mẫu */
} Math_Median_t;

/** @brief Cấu trúc dữ liệu quản lý bộ lọc Kalman 1 chiều */
typedef struct {
    float Q;    /**< Nhiễu quá trình (Process Noise Covariance) */
    float R;    /**< Nhiễu đo đạc từ cảm biến (Measurement Noise Covariance) */
    float P;    /**< Sai số ước lượng hiệp phương sai (Estimation Error Covariance) */
    float K;    /**< Độ lợi Kalman (Kalman Gain) */
    float X;    /**< Giá trị trạng thái ước lượng cuối cùng (Estimated Value) */
} Math_Kalman1D_t;

/**
 * @brief Lọc thô Outlier giới hạn khoảng cách [min_limit, max_limit].
 */
uint16_t Math_Filter_Outlier(uint16_t raw_val, uint16_t min_limit, uint16_t max_limit, uint16_t last_valid_val);

/**
 * @brief Khởi tạo bộ lọc trung vị.
 */
void Math_Filter_MedianInit(Math_Median_t *filter);

/**
 * @brief Xử lý giá trị qua bộ lọc trung vị.
 */
uint16_t Math_Filter_MedianProcess(Math_Median_t *filter, uint16_t new_val);

/**
 * @brief Khởi tạo bộ lọc Kalman 1D.
 */
void Math_Filter_KalmanInit(Math_Kalman1D_t *filter, float q, float r, float initial_p, float initial_x);

/**
 * @brief Thực thi lọc Kalman 1D trên một mẫu đo lường mới.
 */
float Math_Filter_KalmanProcess(Math_Kalman1D_t* filter, float measurement);

#endif /* CDD_FILTER_H */
