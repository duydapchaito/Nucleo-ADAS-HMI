#ifndef MCAL_DIO_H
#define MCAL_DIO_H

#include <stdint.h>

// Định nghĩa mức Logic cho GPIO_PIN_SET/RESET của HAL
typedef enum {
    DIO_LEVEL_LOW = 0,
    DIO_LEVEL_HIGH = 1
} Dio_LevelType;

// Định nghĩa các Kênh (Channel) vật lý
typedef enum {
    DIO_CHAN_BUZZER = 0,
    DIO_CHAN_TRIG_FL,
    DIO_CHAN_TRIG_FR,
    DIO_CHAN_TRIG_RL,
    DIO_CHAN_TRIG_RR,
    DIO_CHAN_MAX
} Dio_ChannelType;

void Mcal_Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);
Dio_LevelType Mcal_Dio_ReadChannel(Dio_ChannelType ChannelId);

#endif /* MCAL_DIO_H */
