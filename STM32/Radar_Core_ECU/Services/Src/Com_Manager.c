/**
 * @file Com_Manager.c
 * @brief Services Com Manager - Đóng gói dữ liệu radar thành khung CAN chuẩn 8-bytes và chuỗi NMEA qua UART.
 */

#include "Com_Manager.h"
#include "Eal_CanIf.h"
#include "Os.h"
#include <stdio.h>

/**
 * @brief Hàm phụ nội bộ: Chia đôi khoảng cách để nén vào 1 byte (Scale 0.5x: 0..510cm -> 0..255).
 * @param dist_cm Khoảng cách thực tế tính bằng cm.
 * @return uint8_t Giá trị sau khi scale (tối đa 255).
 */
static inline uint8_t Com_ScaleDistHalf(uint16_t dist_cm)
{
    uint16_t half = dist_cm >> 1U; /* Chia 2 bằng dịch bit siêu tốc */
    return (half > 255U) ? 255U : (uint8_t)half;
}

/**
 * @brief Đóng gói và truyền khung CAN 0x300 (8 Bytes) chứa trạng thái cần số, 6 khoảng cách và bit lỗi.
 */
Com_StatusType Com_Manager_SendRadarCan(uint8_t GearState, const uint16_t Dist6[6], uint8_t ErrorMask)
{
    if (Dist6 == NULL)  return COM_TX_ERROR;

    uint8_t payload[8];
    payload[0] = GearState;
    for (uint8_t i = 0; i < 6U; i++)
    {
        payload[i + 1U] = Com_ScaleDistHalf(Dist6[i]);  // Bytes 1-6: 6 cảm biến radar
    }
    payload[7] = ErrorMask; // Byte 7: Bitmask lỗi cảm biến và cần số

    CanIf_PduInfoType pdu = {
        .SduDataPtr = payload,
        .SduLength = 8U
    };

    uint8_t res = Eal_CanIf_Transmit(CANIF_TX_PDU_RADAR_STATUS, &pdu);
    if (res == 0U)  return COM_TX_OK;
    if (res == 1U)  return COM_TX_BUSY;
    return COM_TX_ERROR;
}

/**
 * @brief Đóng gói chuỗi NMEA dạng văn bản ($RADAR:...) để debug trực tiếp qua UART2 DMA.
 */
Com_StatusType Com_Manager_SendRadarNmea(char GearChar, const uint16_t Dist6[6])
{
    if (Dist6 == NULL) return COM_TX_ERROR;

    char txBuffer[96];
    int len = snprintf(txBuffer, sizeof(txBuffer), "$RADAR:%c,%u,%u,%u,%u,%u,%u\r\n",
                        GearChar,
                        (unsigned int)Dist6[0], (unsigned int)Dist6[1], (unsigned int)Dist6[2],
                        (unsigned int)Dist6[3], (unsigned int)Dist6[4], (unsigned int)Dist6[5]);
    if (len > 0) {
        Os_Uart_PushRawString(txBuffer, len);
        return COM_TX_OK;
    }
    return COM_TX_ERROR;
}
