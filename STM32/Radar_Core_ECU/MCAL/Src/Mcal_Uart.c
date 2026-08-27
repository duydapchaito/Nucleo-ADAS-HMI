#include "usart.h"
#include "Mcal_Uart.h"

extern UART_HandleTypeDef huart2;

Mcal_Uart_StatusType Mcal_Uart_Transmit(const uint8_t* pData, uint16_t Length)
{
    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart2, (uint8_t*)pData, Length);

    if (status == HAL_OK) {
        return UART_STATUS_OK;
    } else if (status == HAL_BUSY) {
        return UART_STATUS_BUSY;
    }
    return UART_STATUS_ERROR;
}
