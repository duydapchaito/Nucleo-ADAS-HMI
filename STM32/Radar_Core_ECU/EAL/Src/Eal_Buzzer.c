/**
 * @file Eal_Buzzer.c
 * @brief EAL Buzzer Driver - Điều khiển còi báo động dựa trên mức cảnh báo khoảng cách.
 */

#include "Eal_Buzzer.h"
#include "Mcal_Dio.h"

// Trạng thái lưu trữ cục bộ của còi (Dùng cho hàm Toggle)
static Dio_LevelType Buzzer_State = DIO_LEVEL_LOW;

/**
 * @brief Bật còi báo động (Mức điện áp cao).
 */
void Eal_Buzzer_TurnOn(void)
{
    Buzzer_State = DIO_LEVEL_HIGH;
    Mcal_Dio_WriteChannel(DIO_CHAN_BUZZER, DIO_LEVEL_HIGH);
}

/**
 * @brief Tắt còi báo động (Mức điện áp thấp).
 */
void Eal_Buzzer_TurnOff(void)
{
    Buzzer_State = DIO_LEVEL_LOW;
    Mcal_Dio_WriteChannel(DIO_CHAN_BUZZER, DIO_LEVEL_LOW);
}

/**
 * @brief Đảo trạng thái còi (Bật thành Tắt và ngược lại).
 */
void Eal_Buzzer_Toggle(void)
{
    if (Buzzer_State == DIO_LEVEL_HIGH) {
        Eal_Buzzer_TurnOff();
    } else {
        Eal_Buzzer_TurnOn();
    }
}
