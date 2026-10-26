/**
 * @file Cdd_Fusion.c
 * @brief CDD Math Fusion - Xử lý chuỗi lọc tín hiệu và thuật toán hợp thể cảm biến ảo (Virtual Center).
 */

#include "Cdd_Fusion.h"
#include "Cdd_Filter.h"

/* Định nghĩa bitmask lỗi cho 6 cảm biến (Bit 5 đến Bit 0 tương ứng: FL-FC-FR-RL-RC-RR) */
#define FUSION_ERR_BIT_FL   (1U << 5)
#define FUSION_ERR_BIT_FC   (1U << 4)
#define FUSION_ERR_BIT_FR   (1U << 3)
#define FUSION_ERR_BIT_RL   (1U << 2)
#define FUSION_ERR_BIT_RC   (1U << 1)
#define FUSION_ERR_BIT_RR   (1U << 0)

#define US_TO_CM_DIVISOR    58UL    /**< Hằng số quy đổi thời gian Echo (us) sang khoảng cách (cm) cho HC-SR04 */

/* Bộ nhớ trạng thái lọc cho 4 mắt cảm biến phần cứng vật lý */
static Math_Median_t    s_MedianFilters[MATH_HW_SENSORS_COUNT];
static Math_Kalman1D_t  s_KalmanFilters[MATH_HW_SENSORS_COUNT];
static uint16_t         s_LastValidCm[MATH_HW_SENSORS_COUNT];

/**
 * @brief Thuật toán hợp thể hình học (Heuristic Cosine Fusion): Tạo mắt cảm biến ảo ở giữa (FC/RC).
 * @param dist_left Khoảng cách từ mắt bên trái.
 * @param dist_right Khoảng cách từ mắt bên phải.
 * @return uint16_t Khoảng cách hiệu chỉnh của mắt ảo ở giữa.
 */
uint16_t Math_Fusion_VirtualCenter(uint16_t dist_left, uint16_t dist_right)
{
    /*
    * THUẬT TOÁN ĐÁNH GIÁ (Heuristic Fusion)
    * Nếu có 1 vật thể xuất hiện, cảm biến siêu âm vốn có góc mở rộng (dạng nón),
    * nên vật nằm ở giữa chắc chắn sẽ bị quét trúng bởi 2 rìa sóng của cảm biến Trái và Phải.
    * Do bị quét bởi rìa, cạnh huyền luôn dài hơn cạnh góc vuông -> Thực tế vật thế gần hơn số đo được.
    */
    
    uint16_t min_dist = (dist_left < dist_right) ? dist_left : dist_right;

    // Nếu cả 2 cảm biến đều ở xa tầm phát hiện (>200cm), coi như không gian thông thoáng
    if (min_dist >= 200) {
        return min_dist;
    }

    // Hiệu chỉnh góc quét nón sóng siêu âm bằng hệ số 0.85f tối ưu cho phần cứng FPU
    float virtual_dist = ((float)min_dist * 0.85f) + 0.5f;

    return (uint16_t)virtual_dist;
}

/**
 * @brief Khởi tạo toàn bộ bộ lọc và thông số trạng thái cho 4 mắt phần cứng.
 */
void Math_Fusion_Init(uint16_t min_limit, uint16_t max_limit)
{
    (void)min_limit;
    for (uint8_t i = 0; i < MATH_HW_SENSORS_COUNT; i++)
    {
        Math_Filter_MedianInit(&s_MedianFilters[i]);
        /* Khởi tạo Kalman: Q = 0.05f (Nhiễu nhỏ), R = 2.0f (Độ tin cậy cảm biến), P0 = 1.0f, X0 = max_limit */
        Math_Filter_KalmanInit(&s_KalmanFilters[i], 0.05f, 2.0f, 1.0f, (float)max_limit);
        s_LastValidCm[i] = max_limit;
    }
}

/**
 * @brief Hàm phụ trợ: Chạy chuỗi pipeline Outlier -> Median -> Kalman cho một kênh cảm biến độc lập.
 */
static uint16_t Math_Fusion_RunSingleChannel(uint8_t hw_idx,
                                            uint32_t ticks,
                                            uint16_t min_limit,
                                            uint16_t max_limit,
                                            uint8_t *p_is_err)
{
    uint16_t clean_cm;

    if (ticks == 0U) {
        // Mất xung Echo (Timeout / Lỗi phần cứng) -> Bật cờ lỗi và đẩy dần về giới hạn max
        *p_is_err = 1U;
        clean_cm = max_limit;
    } else {
        *p_is_err = 0U;
        uint16_t raw_cm = (uint16_t)(ticks / US_TO_CM_DIVISOR);

        // Bước 1: Lọc thô Outlier
        clean_cm = Math_Filter_Outlier(raw_cm, min_limit, max_limit, s_LastValidCm[hw_idx]);
        s_LastValidCm[hw_idx] = clean_cm;
    }

    // Bước 2: Lọc trung vị Median loại bỏ nhiễu xung
    uint16_t median_cm = Math_Filter_MedianProcess(&s_MedianFilters[hw_idx], clean_cm);

    // Bước 3: Lọc Kalman 1D làm mượt đường đi tín hiệu
    float kalman_cm = Math_Filter_KalmanProcess(&s_KalmanFilters[hw_idx], (float)median_cm);
    uint16_t final_cm = (uint16_t)(kalman_cm + 0.5f);

    // Kẹp giá trị trong biên [min_limit, max_limit] an toàn
    if (final_cm < min_limit)   final_cm = min_limit;
    if (final_cm > max_limit)   final_cm = max_limit;

    return final_cm;
}

/**
 * @brief Thực thi toàn bộ chuỗi Pipeline xử lý mảng radar từ 4 ticks thô ra 6 khoảng cách logic.
 */
void Math_Fusion_ProcessRadarArray(const uint32_t raw_ticks_4[MATH_HW_SENSORS_COUNT],
                                    uint16_t min_limit, uint16_t max_limit,
                                    uint16_t out_dist_6[MATH_LOGIC_SENSORS_COUNT],
                                    uint8_t *out_err_mask_6)
{
    uint8_t err_fl = 0, err_fr = 0, err_rl = 0, err_rr = 0;
    uint8_t mask = 0x00U;

    /* 1. Xử lý lọc qua pipeline cho 4 mắt phần cứng đặt vào vị trí tương ứng: FL(0), FR(2), RL(3), RR(5) */
    out_dist_6[0] = Math_Fusion_RunSingleChannel(0U, raw_ticks_4[0], min_limit, max_limit, &err_fl);
    out_dist_6[2] = Math_Fusion_RunSingleChannel(1U, raw_ticks_4[1], min_limit, max_limit, &err_fr);
    out_dist_6[3] = Math_Fusion_RunSingleChannel(2U, raw_ticks_4[2], min_limit, max_limit, &err_rl);
    out_dist_6[5] = Math_Fusion_RunSingleChannel(3U, raw_ticks_4[3], min_limit, max_limit, &err_rr);

    if (err_fl) mask |= FUSION_ERR_BIT_FL;
    if (err_fr) mask |= FUSION_ERR_BIT_FR;
    if (err_rl) mask |= FUSION_ERR_BIT_RL;
    if (err_rr) mask |= FUSION_ERR_BIT_RR;

    /* 2. Hợp thể mắt ảo phía trước FC (Vị trí 1) */
    if (!err_fl && !err_fr) {
        uint16_t fc = Math_Fusion_VirtualCenter(out_dist_6[0], out_dist_6[2]);
        out_dist_6[1] = (fc < min_limit) ? min_limit : fc;
    } else {
        out_dist_6[1] = max_limit;
        mask |= FUSION_ERR_BIT_FC;
    }

    /* 3. Hợp thể mắt ảo phía sau RC (Vị trí 4) */
    if (!err_rl && !err_rr) {
        uint16_t rc = Math_Fusion_VirtualCenter(out_dist_6[3], out_dist_6[5]);
        out_dist_6[4] = (rc < min_limit) ? min_limit : rc;
    } else {
        out_dist_6[4] = max_limit;
        mask |= FUSION_ERR_BIT_RC;
    }

    // Vá lỗi an toàn: Gán đúng bitmask lỗi ra biến con trỏ out_err_mask_6
    if (out_err_mask_6 != (void*)0) {
        *out_err_mask_6 = mask; 
    }
}
