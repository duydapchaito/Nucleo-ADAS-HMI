/**
 * @file Eal_Radar_Hw.c
 * @brief EAL Radar Hardware - Quản lý kích hoạt xung Trigger, bắt sườn Echo và phát hiện mất xung.
 */

#include "Eal_RadarHw.h"
#include "Mcal_Timer.h"
#include "Mcal_Dio.h"
#include <stdint.h>

#define MAX_RADAR_SENSORS   4U

static Eal_SensorRaw_t Radar_HwData[MAX_RADAR_SENSORS];

// Bảng ánh xạ chân Trigger phần cứng cho 4 mắt siêu âm (FL, FR, RL, RR)
static const Dio_ChannelType Trig_Pins[MAX_RADAR_SENSORS] = {
    DIO_CHAN_TRIG_FL, DIO_CHAN_TRIG_FR, DIO_CHAN_TRIG_RL, DIO_CHAN_TRIG_RR
};

void Eal_Radar_Init(void)
{
    for (uint8_t i = 0U; i < MAX_RADAR_SENSORS; i++)
    {
        Radar_HwData[i].Capture_Val1    = 0U;
        Radar_HwData[i].Capture_Val2    = 0U;
        Radar_HwData[i].Is_First_Edge   = 0U;
        Radar_HwData[i].Is_Data_Ready   = 0U;
        Radar_HwData[i].Delta_Ticks     = 0U;
    }

    Mcal_Timer_RegisterCaptureCallback(Eal_Radar_ProcessCapture);
    Mcal_Timer_Init();
}

/**
 * @brief Kích hoạt xung Trigger dài 10us để phát sóng siêu âm cho một cảm biến cụ thể.
 * @param SensorId Chỉ số mắt cảm biến (0 đến 3).
 */
void Eal_Radar_Trigger(uint8_t SensorId)
{
    if (SensorId >= MAX_RADAR_SENSORS) {
        return;
    }

    /* Đặt lại trạng thái bắt sườn lên để chống kẹt pha nếu chu kỳ trước bị mất Echo */
    Radar_HwData[SensorId].Capture_Val1     = 0U;
    Radar_HwData[SensorId].Capture_Val2     = 0U;
    Radar_HwData[SensorId].Is_First_Edge    = 0U;
    Radar_HwData[SensorId].Is_Data_Ready    = 0U;
    Radar_HwData[SensorId].Delta_Ticks      = 0U;
    Mcal_Timer_SetCapturePolarity(SensorId, 1U);

    Dio_ChannelType pin = Trig_Pins[SensorId];

    Mcal_Dio_WriteChannel(pin, DIO_LEVEL_LOW);
    Mcal_Timer_DelayUs(2U);
    Mcal_Dio_WriteChannel(pin, DIO_LEVEL_HIGH);
    Mcal_Timer_DelayUs(10U);
    Mcal_Dio_WriteChannel(pin, DIO_LEVEL_LOW);
}

/**
 * @brief Xử lý ngắt Input Capture: Đo khoảng thời gian giữa sườn lên và sườn xuống của xung Echo.
 */
void Eal_Radar_ProcessCapture(uint8_t SensorId, uint32_t CapturedValue)
{
    if (SensorId >= MAX_RADAR_SENSORS) {
        return;
    }

    Eal_SensorRaw_t* sensor = &Radar_HwData[SensorId];

    if (sensor->Is_First_Edge == 0U) { // Cạnh lên
        sensor->Capture_Val1 = CapturedValue;
        sensor->Is_First_Edge = 1U;
        // Đảo sườn bắt cạnh xuống
        Mcal_Timer_SetCapturePolarity(SensorId, 0U);
    } else { // Cạnh xuống
        sensor->Capture_Val2 = CapturedValue;

        // Tính Delta Ticks (Xử lý tràn bộ đếm 16-bit 0xFFFF của TIM1)
        sensor->Delta_Ticks = (uint16_t)(sensor->Capture_Val2 - sensor->Capture_Val1);

        sensor->Is_First_Edge = 0U;
        sensor->Is_Data_Ready = 1U;
        // Đảo sườn bắt cạnh lên cho lần quét sau
        Mcal_Timer_SetCapturePolarity(SensorId, 1U);
    }
}

/**
 * @brief Lấy giá trị ticks thô của cảm biến phục vụ cho tầng RTE/CDD.
 */
uint32_t Eal_Radar_GetRawTicks(uint8_t SensorIndex)
{
    if (SensorIndex < MAX_RADAR_SENSORS) {
        if (Radar_HwData[SensorIndex].Is_Data_Ready != 0U) {
            return Radar_HwData[SensorIndex].Delta_Ticks;
        }
    }
    return 0U;
}
