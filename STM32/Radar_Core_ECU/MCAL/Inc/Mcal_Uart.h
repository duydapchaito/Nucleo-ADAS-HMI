#ifndef MCAL_UART_H
#define MCAL_UART_H

#include <stdint.h>

typedef enum {
    UART_STATUS_OK = 0,
    UART_STATUS_ERROR,
    UART_STATUS_BUSY
} Mcal_Uart_StatusType;

// Hàm gửi gói tin
Mcal_Uart_StatusType Mcal_Uart_Transmit(const uint8_t* pData, uint16_t Length);

#endif // MCAL_UART_H
