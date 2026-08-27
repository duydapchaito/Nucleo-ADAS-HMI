#include "Lib_Filter.h"

// --- HÀM PHỤ TRỢ NỘI BỘ ---
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

// =========================================
// THỰC THI CÁC BỘ LỌC
// =========================================

uint16_t Math_Filter_Outlier(uint16_t raw_val, uint16_t min_limit, uint16_t max_limit, uint16_t last_valid_val)
{
    if (raw_val < min_limit || raw_val > max_limit) {
        return last_valid_val; // Nếu ngoài vùng, giữ lại giá trị cũ
    }
    return raw_val;
}

void Math_Filter_MedianInit(Math_Median_t *filter)
{
    filter->Index = 0;
    filter->Is_Filled = 0;
    for (uint8_t i = 0; i < MEDIAN_WINDOW_SIZE; i++) {
        filter->Buffer[i] = 0;
    }
}

uint16_t Math_Filter_MedianProcess(Math_Median_t *filter, uint16_t new_val)
{
    filter->Buffer[filter->Index] = new_val;
    filter->Index++;

    if (filter->Index >= MEDIAN_WINDOW_SIZE) {
        filter->Index = 0;
        filter->Is_Filled = 1;
    }

    if (!filter->Is_Filled) 
        return new_val; // Chưa đủ mẫu thì xuất data thật

    // Copy mảng để ko làm hỏng mảng sort
    uint16_t temp[MEDIAN_WINDOW_SIZE];
    for (uint8_t i = 0; i < MEDIAN_WINDOW_SIZE; i++) {
        temp[i] = filter->Buffer[i];
    }

    Sort_Array(temp, MEDIAN_WINDOW_SIZE);
    return temp[MEDIAN_WINDOW_SIZE / 2];
}

void Math_Filter_KalmanInit(Math_Kalman1D_t *filter, float q, float r, float initial_p, float initial_x)
{
    filter->Q = q;
    filter->R = r;
    filter->P = initial_p;
    filter->X = initial_x;
}

float Math_Filter_KalmanProcess(Math_Kalman1D_t *filter, float measurement)
{
    // 1. Dự đoán
    filter->P = filter->P + filter->Q;

    // 2. Cập nhật - Tính Kalman Gain
    filter->K = filter->P / (filter->P + filter->R);
    filter->X = filter->X + filter->K * (measurement - filter->X);
    filter->P = (1.0f - filter->K) * filter->P;

    return filter->X;
}
