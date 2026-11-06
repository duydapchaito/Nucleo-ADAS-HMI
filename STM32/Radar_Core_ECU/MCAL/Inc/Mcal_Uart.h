/**
 * @file Mcal_Uart.h
 * @brief MCAL UART Header - Định nghĩa API truyền dữ liệu tốc độ cao qua DMA.
 */

#ifndef MCAL_UART_H
#define MCAL_UART_H

#include <stdint.h>

/** @brief Trạng thái truyền dữ liệu qua UART */
typedef enum {
    MCAL_UART_OK          = 0x00U,
    MCAL_UART_ERROR       = 0x01U,
    MCAL_UART_BUSY        = 0x02U
} Mcal_Uart_StatusType;

/** @brief Con trỏ hàm Callback báo hiệu hoàn tất truyền DMA */
typedef void (*Mcal_Uart_TxCpltCbType)(void);

/**
 * @brief Đăng ký hàm Callback khi UART truyền xong chuỗi byte qua DMA.
 */
void Mcal_Uart_RegisterTxCpltCallback(Mcal_Uart_TxCpltCbType cb);

/**
 * @brief Truyền một chuỗi byte qua UART sử dụng phần cứng DMA.
 * @param pData Con trỏ trỏ tới bộ đệm dữ liệu cần truyền.
 * @param Length Kích thước dữ liệu (tính bằng số byte).
 * @return Mcal_Uart_StatusType Trạng thái thực thi.
 */
Mcal_Uart_StatusType Mcal_Uart_Transmit(const uint8_t* pData, uint16_t Length);

/**
 * @brief Truyền chuỗi ký tự trực tiếp bằng cơ chế Polling thanh ghi (Dùng riêng cho Panic/Fault Hook).
 */
void Mcal_Uart_TransmitPolling(const char *pStr);

#endif // MCAL_UART_H
