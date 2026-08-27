#include "Eal_Buzzer.h"
#include "Mcal_Dio.h"

// Biến nội bộ lưu trạng thái để làm hàm Toggle
static Dio_LevelType Buzzer_State = DIO_LEVEL_LOW;

void Eal_Buzzer_TurnOn(void)
{
    Buzzer_State = DIO_LEVEL_HIGH;
    Mcal_Dio_WriteChannel(DIO_CHAN_BUZZER, DIO_LEVEL_HIGH);
}

void Eal_Buzzer_TurnOff(void)
{
    Buzzer_State = DIO_LEVEL_LOW;
    Mcal_Dio_WriteChannel(DIO_CHAN_BUZZER, DIO_LEVEL_LOW);
}

void Eal_Buzzer_Toggle(void)
{
    if (Buzzer_State == DIO_LEVEL_HIGH) {
        Eal_Buzzer_TurnOff();
    } else {
        Eal_Buzzer_TurnOn();
    }
}
