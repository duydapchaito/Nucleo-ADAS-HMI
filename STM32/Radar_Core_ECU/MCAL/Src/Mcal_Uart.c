/**
 * @file Mcal_Uart.c
 * @brief MCAL UART Driver - Giao tiếp truyền nhận dữ liệu tốc độ cao qua DMA.
 */
 
#include "usart.h"
#include "Mcal_Uart.h"
#include "Os.h"

extern UART_HandleTypeDef huart2;

/**
 * @brief Truyền một chuỗi byte qua UART sử dụng phần cứng DMA.
 * @param pData Con trỏ đến bộ đệm dữ liệu cần truyền.
 * @param Length Kích thước dữ liệu (tính bằng số byte).
 * @return Mcal_Uart_StatusType Trạng thái (MCAL_UART_OK / BUSY / ERROR).
 */
Mcal_Uart_StatusType Mcal_Uart_Transmit(const uint8_t* pData, uint16_t Length)
{
    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(&huart2, (uint8_t*)pData, Length);

    if (status == HAL_OK) {
        return MCAL_UART_OK;
    } else if (status == HAL_BUSY) {
        return MCAL_UART_BUSY;
    }
    return MCAL_UART_ERROR;
}

/**
 * @brief Callback ngắt phần cứng: Khi UART2 truyền xong DMA, báo hiệu lên OS để nhả Semaphore.
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2) {
        Os_Uart_TxCpltFromISR();
    }
}
