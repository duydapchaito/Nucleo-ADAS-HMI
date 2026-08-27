#ifndef MCAL_TIMER_H
#define MCAL_TIMER_H

#include <stdint.h>

// Định nghĩa các loại Timer trong hệ thống 
typedef enum {
    TIMER_DELAY = 0,      // TIM7
    TIMER_HEARTBEAT,        // TIM6 (25ms)
    TIMER_ENCODER,          // TIM2
    TIMER_CAPTURE,          // TIM1
} Timer_HardwareType;

// Định ngĩa hàm
void Mcal_Timer_Init(void);
void Mcal_Timer_DelayUs(uint16_t us);
void Mcal_Timer_DelayMs(uint16_t ms);

// API đọc Encoder thay cho __HAL_TIM_GET_COUNTER
uint32_t Mcal_Timer_GetEncoderCount(void);

// API quản lí Input Capture cho Siêu âm
void Mcal_Timer_StartCaptureChannel(uint8_t Channel);
void Mcal_Timer_StopCaptureChannel(uint8_t Channel);
void Mcal_Timer_SetCapturePolarity(uint8_t Channel, uint8_t isRisingEdge);

#endif /* MCAL_TIMER_H */
