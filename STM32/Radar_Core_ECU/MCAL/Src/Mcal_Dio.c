#include "main.h"
#include "Mcal_Dio.h"

// Bảng map (Lookup Table) ánh xạ Channel ảo xuống Port/Pin vật lý
typedef struct {
    GPIO_TypeDef* Port;
    uint16_t Pin;
} Dio_PinMapping_t;

static const Dio_PinMapping_t Dio_PinMapping[DIO_CHAN_MAX] = 
{
    {BUZZ_GPIO_Port, BUZZ_Pin},         		// DIO_CHAN_BUZZER
    {USS_FL_TRIG_GPIO_Port, USS_FL_TRIG_Pin},   // DIO_CHAN_TRIG_FL
    {USS_FR_TRIG_GPIO_Port, USS_FR_TRIG_Pin},   // DIO_CHAN_TRIG_FR
    {USS_RL_TRIG_GPIO_Port, USS_RL_TRIG_Pin},   // DIO_CHAN_TRIG_RL
    {USS_RR_TRIG_GPIO_Port, USS_RR_TRIG_Pin}    // DIO_CHAN_TRIG_RR
};

void Mcal_Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level) 
{
    if (ChannelId < DIO_CHAN_MAX) {
        GPIO_PinState state = (Level == DIO_LEVEL_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(Dio_PinMapping[ChannelId].Port, Dio_PinMapping[ChannelId].Pin, state);
    }
}

Dio_LevelType Mcal_Dio_ReadChannel(Dio_ChannelType ChannelId)
{
    // 1. Chặn lỗi (Boundary Check)
    if (ChannelId < DIO_CHAN_MAX) {
        // 2. Gọi hàm HAL đọc trạng thái từ chân vật lý thông qua bảng Map
        GPIO_PinState state = HAL_GPIO_ReadPin(Dio_PinMapping[ChannelId].Port, Dio_PinMapping[ChannelId].Pin);

        // 3. Ép kiểu trả về theo chuẩn AUTOSAR của hệ thống
        return (state == GPIO_PIN_SET) ? DIO_LEVEL_HIGH : DIO_LEVEL_LOW;
    }

    // Trả về mức LOW (An toàn) nếu ai đó truyền sai ID kênh
    return DIO_LEVEL_LOW;
}
