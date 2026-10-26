/**
 * @file Eal_Buzzer.h
 * @brief EAL Buzzer Header - Định nghĩa các API điều khiển còi cảnh báo.
 */

#ifndef EAL_BUZZER_H
#define EAL_BUZZER_H

#include <stdint.h>

/**
 * @brief Bật còi báo động.
 */
void Eal_Buzzer_TurnOn(void);

/**
 * @brief Tắt còi báo động.
 */
void Eal_Buzzer_TurnOff(void);

/**
 * @brief Đảo trạng thái còi (Dùng cho nhịp bíp ngắt quãng).
 */
void Eal_Buzzer_Toggle(void);

#endif /* EAL_BUZZER_H */
