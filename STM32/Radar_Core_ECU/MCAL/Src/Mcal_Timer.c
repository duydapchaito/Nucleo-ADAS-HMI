#include "main.h"
#include "Mcal_Timer.h"
#include "tim.h"
#include "Eal_RadarHw.h"

// Extern các biến Timer từ tim.c
extern TIM_HandleTypeDef htim1; // Capture
extern TIM_HandleTypeDef htim2; // Encoder
extern TIM_HandleTypeDef htim6; // Heartbeat 25ms
extern TIM_HandleTypeDef htim7; // Delay us

extern volatile uint8_t g_Task_25ms_Flag; // Khai báo kéo cờ từ main.c sang

void Mcal_Timer_Init(void) 
{
    HAL_TIM_Base_Start(&htim7);             // Bật TIM7 cho Polling Delay
    HAL_TIM_Base_Start_IT(&htim6);          // Bật TIM6 cho Heartbeat 25ms
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL); // Bật TIM2 cho Encoder

    Mcal_Timer_StartCaptureChannel(0);
    Mcal_Timer_StartCaptureChannel(1);
    Mcal_Timer_StartCaptureChannel(2);
    Mcal_Timer_StartCaptureChannel(3);
}

void Mcal_Timer_DelayUs(uint16_t us) 
{
    __HAL_TIM_SET_COUNTER(&htim7, 0); // Reset counter
    while (__HAL_TIM_GET_COUNTER(&htim7) < us);
}

void Mcal_Timer_DelayMs(uint16_t ms) 
{
    while (ms--) {
        Mcal_Timer_DelayUs(1000); // Delay 1ms
    }
}

uint32_t Mcal_Timer_GetEncoderCount(void) 
{
    return __HAL_TIM_GET_COUNTER(&htim2);
}

// Hàm bắt Input Capture theo Channel
void Mcal_Timer_StartCaptureChannel(uint8_t Channel)
{
    uint32_t tim_channel;
    switch (Channel) {
        case 0:
            tim_channel = TIM_CHANNEL_1;
            break;
        case 1:
            tim_channel = TIM_CHANNEL_2;
            break;
        case 2:
            tim_channel = TIM_CHANNEL_3;
            break;
        case 3:
            tim_channel = TIM_CHANNEL_4;
            break;
        default:
            return;
    }
    HAL_TIM_IC_Start_IT(&htim1, tim_channel);
}

// Hàm ngừng Input Capture theo Channel
void Mcal_Timer_StopCaptureChannel(uint8_t Channel)
{
    uint32_t tim_channel;
    switch (Channel) {
        case 0:
            tim_channel = TIM_CHANNEL_1;
            break;
        case 1:
            tim_channel = TIM_CHANNEL_2;
            break;
        case 2:
            tim_channel = TIM_CHANNEL_3;
            break;
        case 3:
            tim_channel = TIM_CHANNEL_4;
            break;
        default:
            return;
    }
    HAL_TIM_IC_Stop_IT(&htim1, tim_channel);
} 

// Hàm hỗ trợ đổi sườn xung cho ngắt IC
void Mcal_Timer_SetCapturePolarity(uint8_t Channel, uint8_t isRisingEdge) 
{
    uint32_t polarity = isRisingEdge ? TIM_INPUTCHANNELPOLARITY_RISING : TIM_INPUTCHANNELPOLARITY_FALLING;
    uint32_t tim_channel;

    // Map từ số Kênh (0-3) sang Macro của HAL
    switch (Channel) {
        case 0:
            tim_channel = TIM_CHANNEL_1;
            break;
        case 1:
            tim_channel = TIM_CHANNEL_2;
            break;
        case 2:
            tim_channel = TIM_CHANNEL_3;
            break;
        case 3:
            tim_channel = TIM_CHANNEL_4;
            break;
        default:
            return; // Kênh không hợp lệ
    }
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim1, tim_channel, polarity);
}

// =========================================================================
// 1. NGẮT THỜI GIAN THỰC (OS HEARTBEAT - 25ms)
// =========================================================================
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM6) {
        g_Task_25ms_Flag = 1;
    }
}

// =========================================================================
// 2. NGẮT INPUT CAPTURE (NẰM GỌN TRONG MCAL)
// =========================================================================
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1) {
        uint32_t capture_val = 0;
        uint8_t sensor_id = 0xFF;
        
        switch (htim->Channel) {
            case HAL_TIM_ACTIVE_CHANNEL_1:
                capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
                sensor_id = 0; // FL
                break;
            case HAL_TIM_ACTIVE_CHANNEL_2:
                capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
                sensor_id = 1; // FR
                break;
            case HAL_TIM_ACTIVE_CHANNEL_3:
                capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
                sensor_id = 2; // RL
                break;
            case HAL_TIM_ACTIVE_CHANNEL_4:
                capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
                sensor_id = 3; // RR
                break;
            default:
                break;
        }

        if (sensor_id != 0xFF) {
            Eal_Radar_ProcessCapture(sensor_id, capture_val);
        }
    }
}
