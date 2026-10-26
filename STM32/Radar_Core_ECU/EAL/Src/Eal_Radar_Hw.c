/**
 * @file Eal_Radar_Hw.c
 * @brief EAL Radar Hardware - Quản lý việc kích hoạt xung Trigger và tính toán thời gian Echo thô (Ticks) cho 4 mắt siêu âm.
 */

#include "Eal_RadarHw.h"
#include "Mcal_Timer.h"
#include "Mcal_Dio.h"
#include <stdint.h>

#define MAX_RADAR_SENSORS 4
static Eal_SensorRaw_t Radar_HwData[MAX_RADAR_SENSORS];

// Bảng ánh xạ chân Trigger phần cứng cho 4 mắt siêu âm (FL, FR, RL, RR)
static const Dio_ChannelType Trig_Pins[MAX_RADAR_SENSORS] = {
    DIO_CHAN_TRIG_FL, DIO_CHAN_TRIG_FR, DIO_CHAN_TRIG_RL, DIO_CHAN_TRIG_RR
};

/**
 * @brief Kích hoạt xung Trigger dài 10us để phát sóng siêu âm cho một cảm biến cụ thể.
 * @param SensorId Chỉ số mắt cảm biến (0 đến 3).
 */
void Eal_Radar_Trigger(uint8_t SensorId)
{
    if (SensorId >= MAX_RADAR_SENSORS) return;

    Dio_ChannelType pin = Trig_Pins[SensorId];

    Mcal_Dio_WriteChannel(pin, DIO_LEVEL_LOW);
    Mcal_Timer_DelayUs(2);
    Mcal_Dio_WriteChannel(pin, DIO_LEVEL_HIGH);
    Mcal_Timer_DelayUs(10);
    Mcal_Dio_WriteChannel(pin, DIO_LEVEL_LOW);
}

/**
 * @brief Xử lý ngắt Input Capture: Đo khoảng thời gian giữa sườn lên và sườn xuống của xung Echo.
 */
void Eal_Radar_ProcessCapture(uint8_t SensorId, uint32_t CapturedValue)
{
    if (SensorId >= MAX_RADAR_SENSORS) return;

    Eal_SensorRaw_t* sensor = &Radar_HwData[SensorId];

    if (sensor->Is_First_Edge == 0) { // Cạnh lên
        sensor->Capture_Val1 = CapturedValue;
        sensor->Is_First_Edge = 1;
        // Đảo sườn bắt cạnh xuống
        Mcal_Timer_SetCapturePolarity(SensorId, 0);
    } else { // Cạnh xuống
        sensor->Capture_Val2 = CapturedValue;

        // Tính Delta Ticks (Xử lý tràn bộ đếm 16-bit 0xFFFF của TIM1)
        sensor->Delta_Ticks = (uint16_t)(sensor->Capture_Val2 - sensor->Capture_Val1);

        sensor->Is_First_Edge = 0;
        // Đảo sườn bắt cạnh lên cho lần quét sau
        Mcal_Timer_SetCapturePolarity(SensorId, 1);
    }
}

/**
 * @brief Lấy giá trị ticks thô của cảm biến phục vụ cho tầng RTE/CDD.
 */
uint32_t Eal_Radar_GetRawTicks(uint8_t SensorIndex)
{
    if (SensorIndex < MAX_RADAR_SENSORS) {
        return Radar_HwData[SensorIndex].Delta_Ticks;
    }
    return 0;
}
