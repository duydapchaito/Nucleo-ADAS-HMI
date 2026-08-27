#include "Eal_RadarHw.h"
#include "Mcal_Timer.h"
#include "Mcal_Dio.h"
#include <stdint.h>

// Có 4 cảm biến (FL, FR, RL, RR) map với 4 Timer Channel
// Ở đây làm khung mảng lưu trữ cho ngắt
#define MAX_RADAR_SENSORS 4
static Eal_SensorRaw_t Radar_HwData[MAX_RADAR_SENSORS];

static const Dio_ChannelType Trig_Pins[MAX_RADAR_SENSORS] = {
    DIO_CHAN_TRIG_FL, DIO_CHAN_TRIG_FR, DIO_CHAN_TRIG_RL, DIO_CHAN_TRIG_RR
};

// Hàm bắn Trigger tuần tự 10us
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

// Hàm này sẽ được gọi bên trong callback ngắt Input Capture của MCAL/main
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


// API Getter cho RTE rút data thô
uint32_t Eal_Radar_GetRawTicks(uint8_t SensorIndex)
{
    if (SensorIndex < MAX_RADAR_SENSORS) {
        return Radar_HwData[SensorIndex].Delta_Ticks;
    }
    return 0;
}
