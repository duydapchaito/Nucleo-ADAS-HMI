/**
 * @file Log.c
 * @brief Triển khai BSW Singleton Logger Service.
 */

#include "Log.h"
#include "Mcal_Uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef struct {
    char        buf[LOG_MAX_STR_LEN];
    uint16_t    len;
} Log_MsgType;

/** @brief Cấu trúc Singleton quản lý toàn bộ trạng thái của BSW Logger */
typedef struct {
    QueueHandle_t       queue;
    SemaphoreHandle_t   txDoneSem;
    Log_MsgType         dmaTxBuf;
} Log_SingletonType;

/* Thực thể Singleton duy nhất trong toàn hệ thống */
static Log_SingletonType s_LogInstance = {NULL, NULL, {{0}, 0U}};

/** @brief Callback ngắt phần cứng khi MCAL UART truyền xong khung DMA */
static void Log_UartTxCpltIsrCallback(void)
{
    if (s_LogInstance.txDoneSem == NULL) {
        return;
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    (void)xSemaphoreGiveFromISR(s_LogInstance.txDoneSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void Log_Init(void)
{
    s_LogInstance.queue = xQueueCreate(LOG_QUEUE_LEN, sizeof(Log_MsgType));
    s_LogInstance.txDoneSem = xSemaphoreCreateBinary();

    if (s_LogInstance.txDoneSem != NULL) {
        (void)xSemaphoreGive(s_LogInstance.txDoneSem);
    }

    Mcal_Uart_RegisterTxCpltCallback(Log_UartTxCpltIsrCallback);
}

void Log_TaskLoop(void)
{
    for (;;)
    {
        if (xSemaphoreTake(s_LogInstance.txDoneSem, portMAX_DELAY) == pdTRUE) {
            if (xQueueReceive(s_LogInstance.queue, &s_LogInstance.dmaTxBuf, portMAX_DELAY) == pdPASS) {
                if (Mcal_Uart_Transmit((const uint8_t *)s_LogInstance.dmaTxBuf.buf, s_LogInstance.dmaTxBuf.len) != MCAL_UART_OK) {
                    (void)xSemaphoreGive(s_LogInstance.txDoneSem);
                }
            } else {
                (void)xSemaphoreGive(s_LogInstance.txDoneSem);
            }
        }
    }
}

void Log_PushRawString(const char *pStr, uint16_t len)
{
    if ((s_LogInstance.queue == NULL) || (pStr == NULL) || (len == 0U)) {
        return;
    }

    Log_MsgType msg;
    if (len >= LOG_MAX_STR_LEN) {
        len = LOG_MAX_STR_LEN - 1U;
    }
    memcpy(msg.buf, pStr, len);
    msg.buf[len] = '\0';
    msg.len = len;

    (void)xQueueSend(s_LogInstance.queue, &msg, 0U);
}

void Log_Print(const char *pTag, const char *pFmt, ...)
{
    if ((s_LogInstance.queue == NULL) || (pTag == NULL) || (pFmt == NULL)) {
        return;
    }

    Log_MsgType msg;
    uint32_t tick = xTaskGetTickCount();

    int prefix_len = snprintf(msg.buf, LOG_MAX_STR_LEN, "[%06lu][%s]", (unsigned long)tick, pTag);
    if ((prefix_len < 0) || (prefix_len >= (int)LOG_MAX_STR_LEN)) {
        return;
    }

    va_list args;
    va_start(args, pFmt);
    int body_len = vsnprintf(msg.buf + prefix_len, LOG_MAX_STR_LEN - (uint16_t)prefix_len - 2U, pFmt, args);
    va_end(args);

    if (body_len < 0) {
        return;
    }

    int total_len = prefix_len + body_len;
    if (total_len > ((int)LOG_MAX_STR_LEN - 3)) {
        total_len = (int)LOG_MAX_STR_LEN - 3;
    }

    msg.buf[total_len++]    = '\r';
    msg.buf[total_len++]    = '\n';
    msg.buf[total_len]      = '\0';
    msg.len                 = (uint16_t)total_len;

    (void)xQueueSend(s_LogInstance.queue, &msg, 0U);
}

void Log_FatalPolling(const char *pPrefix, const char *pDetail)
{
    if (pPrefix != NULL) {
        Mcal_Uart_TransmitPolling(pPrefix);
    }
    if (pDetail != NULL) {
        Mcal_Uart_TransmitPolling(pDetail);
    }
    Mcal_Uart_TransmitPolling("\r\n");
}
