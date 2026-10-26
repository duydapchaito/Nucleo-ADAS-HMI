/**
 * @file    Mcal_I2c.c
 * @brief   Triển khai MCAL I2C sử dụng HAL_IT và kiến trúc OS Abstraction.
 */

#include "Mcal_I2c.h"
#include "i2c.h"    /* Chứa khai báo extern I2C_HandleTypeDef hi2c1 do CubeMX sinh ra */
#include "Os.h"

Mcal_I2c_StatusType Mcal_I2c_WriteMem(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef status;

    /* Dọn dẹp trạng thái Semaphore trong OS trước khi truyền */
    Os_I2c_ClearSemaphore();

    /* Bắn lệnh truyền xuống phần cứng (Non-blocking) */
    status = HAL_I2C_Mem_Write_IT(&hi2c1, DevAddress, MemAddress, MemAddSize, pData, Size);

    if (status == HAL_OK) {
        if (Os_I2c_WaitForTransfer(Timeout) == true) {
            return MCAL_I2C_OK;
        } else {
            return MCAL_I2C_TIMEOUT;
        }
    }
    return MCAL_I2C_ERROR;;
}

Mcal_I2c_StatusType Mcal_I2c_ReadMem(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    HAL_StatusTypeDef status;

    Os_I2c_ClearSemaphore();

    status = HAL_I2C_Mem_Read_IT(&hi2c1, DevAddress, MemAddress, MemAddSize, pData, Size);

    if (status == HAL_OK) {
        if (Os_I2c_WaitForTransfer(Timeout) == true) {
            return MCAL_I2C_OK;
        } else {
            return MCAL_I2C_TIMEOUT;
        }
    }
    return MCAL_I2C_ERROR;
}

Mcal_I2c_StatusType Mcal_I2c_IsDeviceReady(uint16_t DevAddress, uint32_t Trials, uint32_t Timeout)
{
    if (HAL_I2C_IsDeviceReady(&hi2c1, DevAddress, Trials, Timeout) == HAL_OK) {
        return MCAL_I2C_OK;
    }
    return MCAL_I2C_ERROR;
}

/* ==================================================================== */
/* CÁC HÀM CALLBACK NGẮT PHẦN CỨNG I2C1 (ỦY QUYỀN ĐỒNG BỘ LÊN OS)       */
/* ==================================================================== */

/**
 * @brief Callback ngắt phần cứng: Hoàn tất ghi bộ nhớ I2C1, đánh thức Task qua OS.
 */
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        Os_I2c_TransferDoneFromISR();
    }
}

/**
 * @brief Callback ngắt phần cứng: Hoàn tất đọc bộ nhớ I2C1, đánh thức Task qua OS.
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        Os_I2c_TransferDoneFromISR();
    }
}

/**
 * @brief Callback ngắt phần cứng: Xảy ra lỗi trên bus I2C1, giải phóng Task đang chờ.
 */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        Os_I2c_TransferDoneFromISR();
    }
}
