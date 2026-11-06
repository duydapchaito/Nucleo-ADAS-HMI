/**
 * @file Os.c
 * @brief Services OS Manager - Quản lý hạt nhân FreeRTOS, khởi tạo Task thời gian thực, Mutex và Queue.
 */

#include "Os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "Mcal_I2c.h"
#include "Mcal_Pwr.h"
#include "Eal_CanIf.h"
#include "Cdd_Eeprom.h"
#include "CanNm/CanNm.h"
#include "CanTp/CanTp.h"
#include "Dcm/Dcm.h"
#include "Rte_Radar.h"
#include "Swc_Radar.h"

#include <string.h>

/* Các đối tượng đồng bộ hóa của OS */
static QueueHandle_t        xCanRxQueue     = NULL;
static SemaphoreHandle_t    xRteMutex       = NULL;
static SemaphoreHandle_t    xNvmMutex       = NULL;
static SemaphoreHandle_t    xI2cDoneSem     = NULL;

/* Các Handle quản lý Task */
static TaskHandle_t xTaskCanRxHandle        = NULL;
static TaskHandle_t xTaskSensorAcqHandle    = NULL;
static TaskHandle_t xTaskSwcRadarHandle     = NULL;
static TaskHandle_t xTaskLoggerHandle       = NULL;

/* Quản lý trạng thái Reset trễ của hệ thống */
static volatile uint8_t s_PendingResetType = 0U;
static volatile uint32_t s_ResetDelayTimerMs = 0U;

void OS_PerformReset(void)
{
    Mcal_Pwr_SystemReset();
}

void Os_Nvm_Lock(void)
{
    if (xNvmMutex != NULL) {
        (void)xSemaphoreTake(xNvmMutex, portMAX_DELAY);
    }
}

void Os_Nvm_Unlock(void)
{
    if (xNvmMutex != NULL) {
        (void)xSemaphoreGive(xNvmMutex);
    }
}

void Os_Rte_Lock(void)
{
    if (xRteMutex != NULL) {
        xSemaphoreTake(xRteMutex, portMAX_DELAY);
    }  
}

void Os_Rte_Unlock(void)
{
    if (xRteMutex != NULL) {
        xSemaphoreGive(xRteMutex);
    }  
}

void Os_CanRx_PushFromISR(uint32_t canId, uint8_t dlc, const uint8_t *pData)
{
    if ((xCanRxQueue == NULL) || (pData == NULL) || (dlc > 8U)) {
        return;
    }

    Os_CanRxMsgType rxMsg;
    rxMsg.CanId = canId;
    rxMsg.Dlc = dlc;
    memcpy(rxMsg.Data, pData, dlc);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    (void)xQueueSendFromISR(xCanRxQueue, &rxMsg, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void vTask_Logger(void *pvParameters)
{
    (void)pvParameters;
    Log_TaskLoop();
}

static void vTask_CanRx(void *pvParameters)
{
    (void)pvParameters;
    Os_CanRxMsgType rxMsg;

    for(;;)
    {
        if (xQueueReceive(xCanRxQueue, &rxMsg, portMAX_DELAY) == pdPASS) {
            Eal_CanIf_RxIndication(rxMsg.CanId, rxMsg.Dlc, rxMsg.Data);
        }
    }
}

static void vTask_SensorAcq(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(25U);

    for (;;)
    {
        Rte_Bsw_MainFunction_25ms();
        CanNm_MainFunction(25U);
        CanTp_MainFunction(25U);
        Dcm_MainFunction(25U);
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

static void vTask_SwcRadar(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(50U);

    Swc_Radar_Init();

    for (;;)
    {
        Swc_Radar_MainFunction_50ms();
        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

void Os_Init(void)
{
    xCanRxQueue     = xQueueCreate(OS_CAN_RX_QUEUE_LEN, sizeof(Os_CanRxMsgType));
    xI2cDoneSem     = xSemaphoreCreateBinary();
    xRteMutex       = xSemaphoreCreateMutex();
    xNvmMutex       = xSemaphoreCreateMutex();

    /* 1. Khởi tạo Singleton Logger Service đầu tiên để sẵn sàng ghi log */
    Log_Init();

    /* 2. Đăng ký Callback đồng bộ cho MCAL I2C */
    const Mcal_I2c_SyncCallbacksType i2cSyncCb = {
        .ClearSync      = Os_I2c_ClearSemaphore,
        .WaitSync       = Os_I2c_WaitForTransfer,
        .SignalFromIsr  = Os_I2c_TransferDoneFromISR
    };
    Mcal_I2c_RegisterSyncCallbacks(&i2cSyncCb);

    /* 3. Khởi tạo tuần tự EAL -> CDD -> RTE -> BSW Services (CanNm, CanTp, Dcm) */
    Rte_Radar_Init();
    Eal_CanIf_Init();
    (void)Cdd_Eeprom_Init();
    CanNm_Init();
    CanTp_Init();
    Dcm_Init();

    /* 4. Tạo các Task thời gian thực */
    (void)xTaskCreate(vTask_CanRx,      "CAN_RX",   OS_STACK_WORDS_CAN_RX,      NULL, OS_PRIO_CAN_RX,        &xTaskCanRxHandle);
    (void)xTaskCreate(vTask_SensorAcq,  "SENS_25",  OS_STACK_WORDS_SENSOR_25MS, NULL, OS_PRIO_SENSOR_25MS,   &xTaskSensorAcqHandle);
    (void)xTaskCreate(vTask_SwcRadar,   "SWC_50",   OS_STACK_WORDS_SWC_50MS,    NULL, OS_PRIO_SWC_50MS,      &xTaskSwcRadarHandle);
    (void)xTaskCreate(vTask_Logger,     "LOGGER",   OS_STACK_WORDS_LOGGER,      NULL, OS_PRIO_LOGGER,        &xTaskLoggerHandle);
}

void vApplicationIdleHook(void)
{
    Mcal_Pwr_EnterSleepWFI();
}

void Os_Task_BswM_Entry(void const * argument)
{
    (void)argument;
    Os_Init();

    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(10000U));
        LOG_I("[BSWM] FreeHeap:%uB | StackMin(W) CAN:%lu SENS:%lu SWC:%lu LOG:%lu | NM:%u",
            (unsigned int)xPortGetFreeHeapSize(),
            (unsigned long)uxTaskGetStackHighWaterMark2(xTaskCanRxHandle),
            (unsigned long)uxTaskGetStackHighWaterMark2(xTaskSensorAcqHandle),
            (unsigned long)uxTaskGetStackHighWaterMark2(xTaskSwcRadarHandle),
            (unsigned long)uxTaskGetStackHighWaterMark2(xTaskLoggerHandle),
            (unsigned int)CanNm_GetState());
    }
}

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
    (void)xTask;
    taskDISABLE_INTERRUPTS();
    Log_FatalPolling("\r\n[FATAL] STACK OVERFLOW: ", (const char *)pcTaskName);
    while (1) {}
}

void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    Log_FatalPolling("\r\n[FATAL] HEAP EXHAUSTED!", NULL);
    while (1) {}
}

void Os_DelayMs(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

bool Os_I2c_WaitForTransfer(uint32_t TimeoutMs)
{
    if (xI2cDoneSem != NULL) {
        if (xSemaphoreTake(xI2cDoneSem, pdMS_TO_TICKS(TimeoutMs)) == pdTRUE) {
            return true;
        }
    }
    return false;
}

void Os_I2c_ClearSemaphore(void)
{
    if (xI2cDoneSem != NULL) {
        xSemaphoreTake(xI2cDoneSem, 0U);
    }
}

void Os_I2c_TransferDoneFromISR(void)
{
    if (xI2cDoneSem == NULL) {
        return;
    }

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    (void)xSemaphoreGiveFromISR(xI2cDoneSem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
