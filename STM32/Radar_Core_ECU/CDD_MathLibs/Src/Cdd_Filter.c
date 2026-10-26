/**
 * @file Cdd_Filter.c
 * @brief CDD Math Filter - Hiện thực các bộ lọc tín hiệu số (Outlier, Median, Kalman 1D).
 */

#include "Cdd_Filter.h"

/**
 * @brief Hàm phụ trợ nội bộ: Sắp xếp mảng số nguyên tăng dần (Dùng cho bộ lọc trung vị).
 * @param arr Con trỏ trỏ tới mảng cần sắp xếp.
 * @param size Kích thước mảng.
 */
static void Sort_Array(uint16_t *arr, uint8_t size)
{
    for (uint8_t i = 0; i < size; i++) {
        for (uint8_t j = 0; j < size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                uint16_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

/**
 * @brief Lọc thô Outlier: Loại bỏ các giá trị đo nằm ngoài biên vật lý [min, max].
 * @param raw_val Giá trị đo thô mới nhận.
 * @param min_limit Giới hạn khoảng cách tối thiểu (cm).
 * @param max_limit Giới hạn khoảng cách tối đa (cm).
 * @param last_valid_val Giá trị hợp lệ gần nhất trước đó.
 * @return uint16_t Giá trị sau khi qua tầng lọc thô.
 */
uint16_t Math_Filter_Outlier(uint16_t raw_val, uint16_t min_limit, uint16_t max_limit, uint16_t last_valid_val)
{
    if (raw_val < min_limit || raw_val > max_limit) {
        return last_valid_val; // Giữ lại giá trị cũ nếu dữ liệu thô bị sốc nhiệt/nhiễu biên
    }
    return raw_val;
}

/**
 * @brief Khởi tạo trạng thái cho bộ lọc trượt trung vị (Median Filter).
 */
void Math_Filter_MedianInit(Math_Median_t *filter)
{
    filter->Index = 0;
    filter->Is_Filled = 0;
    for (uint8_t i = 0; i < MEDIAN_WINDOW_SIZE; i++) {
        filter->Buffer[i] = 0;  
    }
}

/**
 * @brief Xử lý giá trị qua cửa sổ trượt trung vị để loại bỏ nhiễu xung (Spike Noise).
 */
uint16_t Math_Filter_MedianProcess(Math_Median_t *filter, uint16_t new_val)
{
    filter->Buffer[filter->Index] = new_val;
    filter->Index++;

    if (filter->Index >= MEDIAN_WINDOW_SIZE) {
        filter->Index = 0;
        filter->Is_Filled = 1;
    }

    if (!filter->Is_Filled) 
        return new_val; // Chưa đủ mẫu trong cửa sổ thì trả về dữ liệu thực

    uint16_t temp[MEDIAN_WINDOW_SIZE];
    for (uint8_t i = 0; i < MEDIAN_WINDOW_SIZE; i++) {
        temp[i] = filter->Buffer[i];
    }

    Sort_Array(temp, MEDIAN_WINDOW_SIZE);
    return temp[MEDIAN_WINDOW_SIZE / 2];    // Lấy giá trị ở giữa sau khi sắp xếp
}

/**
 * @brief Khởi tạo thông số cho bộ lọc Kalman 1 chiều.
 */
void Math_Filter_KalmanInit(Math_Kalman1D_t *filter, float q, float r, float initial_p, float initial_x)
{
    filter->Q = q;
    filter->R = r;
    filter->P = initial_p;
    filter->X = initial_x;
}

/**
 * @brief Thực thi vòng lặp Dự đoán & Cập nhật của bộ lọc Kalman 1 chiều.
 */
float Math_Filter_KalmanProcess(Math_Kalman1D_t *filter, float measurement)
{
    // Bước 1: Dự đoán trạng thái kế tiếp (Prediction)
    filter->P = filter->P + filter->Q;

    // Bước 2: Cập nhật - Tính độ lợi Kalman (Kalman Gain)
    filter->K = filter->P / (filter->P + filter->R);
    filter->X = filter->X + filter->K * (measurement - filter->X);
    filter->P = (1.0f - filter->K) * filter->P;

    return filter->X;
}
