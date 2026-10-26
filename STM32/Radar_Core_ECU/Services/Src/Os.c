/**
 * @file Os.c
 * @brief Services OS Manager - Quản lý hạt nhân FreeRTOS, khởi tạo Task thời gian thực, Mutex và Queue.
 */

#include "Os.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "Mcal_Timer.h"
#include "Mcal_Uart.h"
#include "Mcal_Pwr.h"

#include "Eal_CanIf.h"
#include "Eal_RadarHw.h"
#include "Eal_Encoder.h"

#include "Cdd_Filter.h"
#include "Cdd_Fusion.h"
#include "Cdd_Eeprom.h"

#include "Rte_Radar.h"
#include "Swc_Radar.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// Định nghĩa các đối tượng đồng bộ hóa nhân hệ điều hành
static QueueHandle_t        xCanRxQueue     = NULL;
static QueueHandle_t        xLogQueue       = NULL;
static SemaphoreHandle_t    xUartTxDoneSem  = NULL;
static SemaphoreHandle_t    xRteMutex       = NULL;
static SemaphoreHandle_t    xI2cDoneSem     = NULL;

// Định nghĩa các Handle quản lý Task
static TaskHandle_t xTaskCanRxHandle        = NULL;
static TaskHandle_t xTaskSensorAcqHandle    = NULL;
static TaskHandle_t xTaskSwcRadarHandle     = NULL;
static TaskHandle_t xTaskLoggerHandle       = NULL;

/**
 * @brief Khóa Mutex bảo vệ vùng nhớ Shadow Buffers của RTE.
 */
void Os_Rte_Lock(void)
{
    if (xRteMutex != NULL)  xSemaphoreTake(xRteMutex, portMAX_DELAY);
}

/**
 * @brief Mở khóa Mutex RTE.
 */
void Os_Rte_Unlock(void)
{
    if (xRteMutex != NULL)  xSemaphoreGive(xRteMutex);
}

/**
 * @brief Đẩy khung CAN từ ngắt RX vào Queue an toàn đa nhiệm.
 */
void Os_CanRx_PushFromISR(const Mcal_Can_FrameType *pFrame)
{
    if (xCanRxQueue == NULL || pFrame == NULL)  return;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(xCanRxQueue, pFrame, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief Giải phóng Semaphore từ ngắt MCAL UART để tiếp tục chuỗi log tiếp theo.
 */
void Os_Uart_TxCpltFromISR(void)
{
    if (xUartTxDoneSem == NULL) return;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xUartTxDoneSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief Đẩy chuỗi dữ liệu thô vào hàng đợi Logger để truyền qua UART DMA.
 */
void Os_Uart_PushRawString(const char *str, uint16_t len)
{
    if (xLogQueue == NULL || str == NULL || len == 0U)  return;
    
    Os_LogMsgType msg;
    if (len >= OS_LOG_MAX_STR)  len = OS_LOG_MAX_STR - 1U;
    memcpy(msg.buf, str, len);
    msg.buf[len] = '\0';
    msg.len = len;
    xQueueSend(xLogQueue, &msg, 0);
}

/**
 * @brief In log định dạng định danh (I/W/E) kèm Tick thời gian hệ thống.
 */
void Os_Log_Print(const char *tag, const char *fmt, ...)
{
    if (xLogQueue == NULL)  return;

    Os_LogMsgType msg;
    uint32_t tick = xTaskGetTickCount();

    int prefix_len = snprintf(msg.buf, OS_LOG_MAX_STR, "[%06lu][%s]", (unsigned long)tick, tag);
    if (prefix_len < 0 || prefix_len >= (int)OS_LOG_MAX_STR)    return;

    va_list args;
    va_start(args, fmt);
    int body_len = vsnprintf(msg.buf + prefix_len, OS_LOG_MAX_STR - prefix_len - 2, fmt, args);
    va_end(args);
    if (body_len < 0)   return;

    int total_len = prefix_len + body_len;
    if (total_len > (int)OS_LOG_MAX_STR - 3)    total_len = (int)OS_LOG_MAX_STR - 3;

    msg.buf[total_len++] = '\r';
    msg.buf[total_len++] = '\n';
    msg.buf[total_len] = '\0';
    msg.len = (uint16_t)total_len;
    
    xQueueSend(xLogQueue, &msg, 0);
}

/**
 * @brief Task Logger: Đọc hàng đợi log và điều phối truyền UART DMA an toàn.
 */
static void vTask_Logger(void *pvParameters)
{
    (void)pvParameters;
    static Os_LogMsgType tx_dma_buf;
    for (;;)
    {
        if (xSemaphoreTake(xUartTxDoneSem, portMAX_DELAY) == pdTRUE) {
            if (xQueueReceive(xLogQueue, &tx_dma_buf, portMAX_DELAY) == pdPASS) {
                if (Mcal_Uart_Transmit((const uint8_t *)tx_dma_buf.buf, tx_dma_buf.len) != MCAL_UART_OK) {
                    xSemaphoreGive(xUartTxDoneSem);
                }
            } else {
                xSemaphoreGive(xUartTxDoneSem);
            }
        }
    }
}

/**
 * @brief Task xử lý thông điệp CAN nhận được từ Queue.
 */
static void vTask_CanRx(void *pvParameters)
{
    (void)pvParameters;
    Mcal_Can_FrameType rxFrame;
    for(;;)
    {
        if (xQueueReceive(xCanRxQueue, &rxFrame, portMAX_DELAY) == pdPASS) {
            Eal_CanIf_RxIndication(rxFrame.StdId, rxFrame.Dlc, rxFrame.Data);
        }
    }
}

/**
 * @brief Task BSW 25ms: Đọc ticks thô cảm biến -> Chạy bộ lọc/Fusion -> Đọc Encoder cần số -> Bắn Trigger tiếp theo.
 */
static void vTask_SensorAcq(void *pvParameters)
{
    (void)pvParameters;
    uint8_t current_hw_sensor = 0;
    uint32_t raw_ticks_4[RTE_MAX_HW_SENSORS];
    uint16_t unified_dist_6[RTE_MAX_SENSORS];
    uint8_t sensor_err_mask_6 = 0;
    
    // Bắn mồi Trigger cho mắt đầu tiên trước khi vào vòng lặp thời gian thực
    Eal_Radar_Trigger(current_hw_sensor);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(25);
    
    for (;;)
    {
        // 1. Thu thập Ticks từ phần cứng và ghi vào RTE
        uint32_t ticks = Eal_Radar_GetRawTicks(current_hw_sensor);
        Rte_Write_RawTicks((Rte_HwSensorId_t)current_hw_sensor, ticks);

        // 2. Đọc toàn bộ Ticks -> Chạy thuật toán Math Fusion -> Ghi mảng 6 cảm biến vào RTE
        Rte_Read_AllRawTicks(raw_ticks_4);
        Math_Fusion_ProcessRadarArray(raw_ticks_4, 
                                    Rte_Prm_GetMinSensorRange(), 
                                    Rte_Prm_GetMaxSensorRange(), 
                                    unified_dist_6, 
                                    &sensor_err_mask_6);
        Rte_Write_AllDistancesAndErr(unified_dist_6, sensor_err_mask_6);

        // 3. Quét tín hiệu Encoder điều chỉnh trạng thái Cần số (P, R, N, D)
        int32_t enc_delta = Eal_Encoder_GetDelta();
        if (enc_delta != 0) {
            int32_t gear = (int32_t)Rte_Read_Gear() + enc_delta;
            if (gear < (int32_t)RTE_GEAR_P) gear = (int32_t)RTE_GEAR_P;
            if (gear > (int32_t)RTE_GEAR_D) gear = (int32_t)RTE_GEAR_D;
            Rte_Write_Gear((Rte_GearType)gear);
        }

        // 4. Chuyển sang mắt cảm biến vật lý kế tiếp và kích hoạt Trigger
        current_hw_sensor = (current_hw_sensor + 1U) % (uint8_t)RTE_MAX_HW_SENSORS;
        Eal_Radar_Trigger(current_hw_sensor);

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/**
 * @brief Task Application 50ms: Chạy logic trung tâm của Swc_Radar (Cảnh báo vùng & điều khiển còi).
 */
static void vTask_SwcRadar(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(50);

    Swc_Radar_Init();

    for (;;)
    {
        Swc_Radar_MainFunction_50ms();
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/**
 * @brief Khởi tạo toàn bộ tài nguyên OS (Queue, Semaphore, Mutex) và tạo các Task thời gian thực.
 */
void Os_Init()
{
    xCanRxQueue     = xQueueCreate(8U, sizeof(Mcal_Can_FrameType));
    xLogQueue       = xQueueCreate(OS_LOG_QUEUE_LEN, sizeof(Os_LogMsgType));
    xUartTxDoneSem  = xSemaphoreCreateBinary();
    xI2cDoneSem     = xSemaphoreCreateBinary();
    xRteMutex       = xSemaphoreCreateMutex();

    xSemaphoreGive(xUartTxDoneSem);

    // Khởi tạo các thư viện toán học và phần cứng tầng dưới
    Math_Fusion_Init(Rte_Prm_GetMinSensorRange(), Rte_Prm_GetMaxSensorRange());
    Mcal_Timer_Init();
    Eal_Encoder_Init();
    Eal_CanIf_Init();

    /* Khởi tạo và kiểm tra kết nối vật lý EEPROM AT24C02 */
    (void)Cdd_Eeprom_Init();

    // Tạo các Task với mức ưu tiên được phân cấp chặt chẽ
    xTaskCreate(vTask_CanRx, "CAN_RX", 256, NULL, tskIDLE_PRIORITY + 3, &xTaskCanRxHandle);
    xTaskCreate(vTask_SensorAcq, "SENS_25", 256, NULL, tskIDLE_PRIORITY + 2, &xTaskSensorAcqHandle);
    xTaskCreate(vTask_SwcRadar, "SWC_50", 384, NULL, tskIDLE_PRIORITY + 1, &xTaskSwcRadarHandle);
    xTaskCreate(vTask_Logger, "LOGGER", 384, NULL, tskIDLE_PRIORITY, &xTaskLoggerHandle);
}

/**
 * @brief FreeRTOS Idle Hook: Đưa MCU vào chế độ WFI tiết kiệm điện khi rảnh rỗi.
 */
void vApplicationIdleHook(void)
{
    Mcal_Pwr_EnterSleepWFI();
}

/**
 * @brief Task BSW Manager gốc: Điểm nhảy đầu tiên khi Kernel khởi động, định kỳ in báo cáo Heap/Stack.
 */
void Os_Task_BswM_Entry(void const * argument)
{
    (void)argument;
    Os_Init();
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(10000));
        OS_LOG_I("[BSWM] FreeHeap:%uB | StackMin(W) CAN:%lu SENS:%lu SWC:%lu LOG:%lu",
                (unsigned int)xPortGetFreeHeapSize(),
                (unsigned long)uxTaskGetStackHighWaterMark2(xTaskCanRxHandle),
                (unsigned long)uxTaskGetStackHighWaterMark2(xTaskSensorAcqHandle),
                (unsigned long)uxTaskGetStackHighWaterMark2(xTaskSwcRadarHandle),
                (unsigned long)uxTaskGetStackHighWaterMark2(xTaskLoggerHandle));
    }
}

/**
 * @brief Hook giám sát tràn bộ nhớ Stack của Task.
 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
    (void)xTask;
    __disable_irq();
    const char *msg = "\r\n[FATAL] STACK OVERFLOW: ";
    while (*msg) 
    {
        while (!(USART2->SR & USART_SR_TXE)) {}
        USART2->DR = *msg++;
    }
    while(*pcTaskName)
    {
        while (!(USART2->SR & USART_SR_TXE)) {}
        USART2->DR = *pcTaskName++;
    }
    while (1) {}
}

/**
 * @brief Hook báo động khi không đủ bộ nhớ Heap cấp phát cho hệ điều hành.
 */
void vApplicationMallocFailedHook(void)
{
    __disable_irq();
    const char *msg = "\r\n[FATAL] HEAP EXHAUSTED!\r\n";
    while (*msg) 
    {
        while (!(USART2->SR & USART_SR_TXE)) {}
        USART2->DR = *msg++;
    }
    while (1) {}
}

/* ==================================================================== */
/* OSAL IMPLEMENTATION - QUẢN LÝ TIẾN TRÌNH & ĐỒNG BỘ HÓA I2C           */
/* ==================================================================== */

/**
 * @brief Đưa Task vào trạng thái ngủ dựa trên Tick của hệ điều hành.
 */
void Os_DelayMs(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

/**
 * @brief Chờ cờ giải phóng từ ngắt I2C, giải phóng CPU trong thời gian chờ.
 */
bool Os_I2c_WaitForTransfer(uint32_t TimeoutMs)
{
    if (xI2cDoneSem != NULL) {
        if (xSemaphoreTake(xI2cDoneSem, pdMS_TO_TICKS(TimeoutMs)) == pdTRUE) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Xóa cờ trạng thái Semaphore phục vụ chuẩn bị phiên truyền mới.
 */
void Os_I2c_ClearSemaphore(void)
{
    if (xI2cDoneSem != NULL) {
        xSemaphoreTake(xI2cDoneSem, 0);
    }
}

/**
 * @brief Đánh thức Task từ ISR khi phần cứng hoàn tất giao dịch dữ liệu I2C.
 */
void Os_I2c_TransferDoneFromISR(void)
{
    if (xI2cDoneSem == NULL)    return;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xI2cDoneSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
