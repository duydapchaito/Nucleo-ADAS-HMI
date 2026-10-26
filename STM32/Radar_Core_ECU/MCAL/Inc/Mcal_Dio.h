/**
 * @file Mcal_Dio.h
 * @brief MCAL DIO Header - Trừu tượng hóa việc quản lý các kênh GPIO kỹ thuật số.
 */

#ifndef MCAL_DIO_H
#define MCAL_DIO_H

#include <stdint.h>

/** @brief Mức điện áp logic của chân GPIO */
typedef enum {
    DIO_LEVEL_LOW = 0,
    DIO_LEVEL_HIGH = 1
} Dio_LevelType;

/** @brief Danh mục các kênh DIO phần cứng được định nghĩa trong hệ thống Radar */
typedef enum {
    DIO_CHAN_BUZZER = 0,
    DIO_CHAN_TRIG_FL,
    DIO_CHAN_TRIG_FR,
    DIO_CHAN_TRIG_RL,
    DIO_CHAN_TRIG_RR,
    DIO_CHAN_MAX
} Dio_ChannelType;

/**
 * @brief Ghi mức điện áp lên một kênh DIO bất kỳ.
 * @param ChannelId ID kênh phần cứng ảo.
 * @param Level Mức điện áp muốn ghi (HIGH/LOW).
 */
void Mcal_Dio_WriteChannel(Dio_ChannelType ChannelId, Dio_LevelType Level);

/**
 * @brief Đọc trạng thái mức điện áp từ một kênh DIO.
 * @param ChannelId ID kênh phần cứng ảo.
 * @return Dio_LevelType Mức điện áp hiện tại của chân.
 */
Dio_LevelType Mcal_Dio_ReadChannel(Dio_ChannelType ChannelId);

#endif /* MCAL_DIO_H */
