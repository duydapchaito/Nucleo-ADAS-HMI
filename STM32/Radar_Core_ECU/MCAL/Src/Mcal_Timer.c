/**
 * @file Mcal_Timer.c
 * @brief MCAL Timer Driver - Quản lý Input Capture cho siêu âm, Encoder cho cần số và hàm tạo trễ us.
 */
 
#include "main.h"
#include "Mcal_Timer.h"
#include "tim.h"
#include "Eal_RadarHw.h"

// Extern các biến Timer từ tim.c
extern TIM_HandleTypeDef htim1; // Capture
extern TIM_HandleTypeDef htim2; // Encoder

/**
 * @brief Khởi tạo các bộ định thời cơ sở, Encoder và bật ngắt Input Capture cho 4 cảm biến.
 */
void Mcal_Timer_Init(void) 
{
    /* Khởi động bộ đếm nền 1MHz của TIM1 và Encoder TIM2 */
    HAL_TIM_Base_Start(&htim1);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL); // Bật TIM2 cho Encoder

    /* Bật ngắt Input Capture cho cả 4 cảm biến FL, FR, RL, RR */
    for (uint8_t ch = 0; ch < 4U; ch++) 
    {
        Mcal_Timer_StartCaptureChannel(ch);
    }
}

/**
 * @brief Tạo khoảng trễ chính xác theo đơn vị micro-giây (Dùng cho việc kích xung Trigger 10us).
 */
void Mcal_Timer_DelayUs(uint16_t us) 
{
    uint16_t start = (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);
    while ((uint16_t)((uint16_t)__HAL_TIM_GET_COUNTER(&htim1) - start) < us)
    {
        __NOP();
    }
}

/**
 * @brief Đọc giá trị xung đếm hiện tại từ Encoder (cần số).
 */
uint32_t Mcal_Timer_GetEncoderCount(void) 
{
    return __HAL_TIM_GET_COUNTER(&htim2);
}

/**
 * @brief Bật ngắt Input Capture theo từng kênh định sẵn.
 */
void Mcal_Timer_StartCaptureChannel(uint8_t Channel)
{
    static const uint32_t channel_map[4] = {
        TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4
    };
    if (Channel < 4U) {
        HAL_TIM_IC_Start_IT(&htim1, channel_map[Channel]);
    }
}

/**
 * @brief Tắt ngắt Input Capture theo kênh.
 */
void Mcal_Timer_StopCaptureChannel(uint8_t Channel)
{
    static const uint32_t channel_map[4] = {
        TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4
    };
    if (Channel < 4U) {
        HAL_TIM_IC_Stop_IT(&htim1, channel_map[Channel]);
    }
} 

/**
 * @brief Cấu hình sườn xung bắt tín hiệu (Cạnh lên / Cạnh xuống) cho Input Capture.
 */
void Mcal_Timer_SetCapturePolarity(uint8_t Channel, uint8_t isRisingEdge) 
{
    static const uint32_t channel_map[4] = {
        TIM_CHANNEL_1, TIM_CHANNEL_2, TIM_CHANNEL_3, TIM_CHANNEL_4
    };
    if (Channel < 4U) {
        uint32_t pol = isRisingEdge ? TIM_INPUTCHANNELPOLARITY_RISING
                                        : TIM_INPUTCHANNELPOLARITY_FALLING;

        // Đảm bảo vô hiệu hóa ngắt tạm thời và xóa cờ sướng xung để tránh kẹt băng thông phần cứng
        __HAL_TIM_DISABLE_IT(&htim1, (TIM_IT_CC1 << Channel));
        TIM_RESET_CAPTUREPOLARITY(&htim1, channel_map[Channel]);
        TIM_SET_CAPTUREPOLARITY(&htim1, channel_map[Channel], pol);
        __HAL_TIM_CLEAR_FLAG(&htim1, (TIM_FLAG_CC1 << Channel) | (TIM_FLAG_CC1OF << Channel));
        __HAL_TIM_ENABLE_IT(&htim1, (TIM_IT_CC1 << Channel));
    }
}

/**
 * @brief Trạm trung chuyển ngắt TIM1: Lọc giá trị captured từ các kênh và chuyển về Eal_RadarHw.
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) {
        uint32_t capture_val = 0;
        uint8_t sensor_id = 0xFF;
        
        switch (htim->Channel) {
            case HAL_TIM_ACTIVE_CHANNEL_1:
                capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                sensor_id = 0; /* FL */
                break;
            case HAL_TIM_ACTIVE_CHANNEL_2:
                capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
                sensor_id = 1; /* FR */
                break;
            case HAL_TIM_ACTIVE_CHANNEL_3:
                capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
                sensor_id = 2; /* RL */
                break;
            case HAL_TIM_ACTIVE_CHANNEL_4:
                capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
                sensor_id = 3; /* RR */
                break;
            default:
                break;
        }

        if (sensor_id != 0xFF) {
            Eal_Radar_ProcessCapture(sensor_id, capture_val);
        }
    }
}
