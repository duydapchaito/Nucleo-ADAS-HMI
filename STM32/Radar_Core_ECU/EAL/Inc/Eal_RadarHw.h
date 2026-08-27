#ifndef EAL_RADARHW_H
#define EAL_RADARHW_H

#include <stdint.h>
#include "Mcal_Dio.h"

// Cấu trúc dữ liệu thô của 1 cảm biến
typedef struct {
    uint32_t Capture_Val1;
    uint32_t Capture_Val2;
    uint8_t Is_First_Edge;
    uint32_t Delta_Ticks; // Dữ liệu quý giá nhất
} Eal_SensorRaw_t;

// Thông số đặc tả vật lý của cảm biến HC-SR04
#define EAL_RADAR_MIN_RANGE_CM  2
#define EAL_RADAR_MAX_RANGE_CM  400

void Eal_Radar_Trigger(uint8_t SensorId);
void Eal_Radar_ProcessCapture(uint8_t SensorId, uint32_t CapturedValue);
uint32_t Eal_Radar_GetRawTicks(uint8_t SensorId);

#endif /* EAL_RADARHW_H */
