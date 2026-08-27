#include "Com_Uart_If.h"
#include "Mcal_Uart.h"
#include <stdio.h>

// Buffer tĩnh để chứa chuỗi (tránh khai báo mảng trong hàm làm tràn Stack)
static char txBuffer[128];

Com_StatusType Com_SendRadarNmeaMessage(char GearState, 
                                        uint16_t DistFL, uint16_t DistFC, uint16_t DistFR, 
                                        uint16_t DistRL, uint16_t DistRC, uint16_t DistRR)
{
    // Đóng gói theo chuẩn NMEA giả lập: $RADAR:[GEAR],[FL],[FC],[FR],[RL],[RC],[RR]
    int len = sprintf(txBuffer, "$RADAR:%c,%d,%d,%d,%d,%d,%d\r\n",
                    GearState,
                    (int)DistFL, (int)DistFC, (int)DistFR,
                    (int)DistRL, (int)DistRC, (int)DistRR);

    if (len > 0) {
        // Gọi thẳng xuống MCAL để truyền đi
        Mcal_Uart_StatusType mcal_status = Mcal_Uart_Transmit((const uint8_t*)txBuffer, (uint16_t)len);

        if (mcal_status == UART_STATUS_OK) {
            return COM_TX_OK;
        } else if (mcal_status == UART_STATUS_BUSY) {
            return COM_TX_BUSY;
        }
    }

    return COM_TX_ERROR;
}
