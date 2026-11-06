/**
 * @file    Mcal_I2c.c
 * @brief   Triển khai MCAL I2C sử dụng HAL_IT và cơ chế Callback đồng bộ chuẩn AUTOSAR.
 */

#include "Mcal_I2c.h"
#include "i2c.h"    /* Chứa khai báo extern I2C_HandleTypeDef hi2c1 do CubeMX sinh ra */
#include <stddef.h>

static Mcal_I2c_SyncCallbacksType s_SyncCb = {NULL, NULL, NULL};
static volatile bool s_I2cTransferError = false;

void Mcal_I2c_RegisterSyncCallbacks(const Mcal_I2c_SyncCallbacksType *pCallbacks)
{
    if (pCallbacks != NULL) {
        s_SyncCb = *pCallbacks;
    }
}

Mcal_I2c_StatusType Mcal_I2c_WriteMem(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    if (s_SyncCb.ClearSync != NULL) {
        s_SyncCb.ClearSync();
    }
    s_I2cTransferError = false;

    /* Bắn lệnh truyền xuống phần cứng (Non-blocking) */
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write_IT(&hi2c1, DevAddress, MemAddress, MemAddSize, pData, Size);

    if (status == HAL_OK) {
        if ((s_SyncCb.WaitSync != NULL) && (s_SyncCb.WaitSync(Timeout) == true)) {
            return s_I2cTransferError ? MCAL_I2C_ERROR : MCAL_I2C_OK;
        }
        return MCAL_I2C_TIMEOUT;
    } else if (status == HAL_BUSY) {
        return MCAL_I2C_BUSY;
    }
    return MCAL_I2C_ERROR;
}

Mcal_I2c_StatusType Mcal_I2c_ReadMem(uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    if (s_SyncCb.ClearSync != NULL) {
        s_SyncCb.ClearSync();
    }
    s_I2cTransferError = false;

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read_IT(&hi2c1, DevAddress, MemAddress, MemAddSize, pData, Size);

    if (status == HAL_OK) {
        if ((s_SyncCb.WaitSync != NULL) && (s_SyncCb.WaitSync(Timeout) == true)) {
            return s_I2cTransferError ? MCAL_I2C_ERROR : MCAL_I2C_OK;
        } else {
            return MCAL_I2C_TIMEOUT;
        }
    } else if (status == HAL_BUSY) {
        return MCAL_I2C_BUSY;
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
    if ((hi2c->Instance == I2C1) && (s_SyncCb.SignalFromIsr != NULL)) {
        s_SyncCb.SignalFromIsr();
    }
}

/**
 * @brief Callback ngắt phần cứng: Hoàn tất đọc bộ nhớ I2C1, đánh thức Task qua OS.
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if ((hi2c->Instance == I2C1) && (s_SyncCb.SignalFromIsr != NULL)) {
        s_SyncCb.SignalFromIsr();
    }
}

/**
 * @brief Callback ngắt phần cứng: Xảy ra lỗi trên bus I2C1, giải phóng Task đang chờ.
 */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance == I2C1) {
        s_I2cTransferError = true;
        if (s_SyncCb.SignalFromIsr != NULL) {
            s_SyncCb.SignalFromIsr();
        }
    }
}
