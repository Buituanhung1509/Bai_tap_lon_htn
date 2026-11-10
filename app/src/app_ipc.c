/**
 * @file    app_ipc.c
 * @brief   Tao cac doi tuong IPC dung chung (goi 1 lan truoc khi tao task).
 */
#include "app_ipc.h"
#include "app_cfg.h"
#include <string.h>

QueueHandle_t     xSensorQueue  = NULL;
QueueHandle_t     xDisplayQueue = NULL;
QueueHandle_t     xUartRxQueue  = NULL;
SemaphoreHandle_t xUartMutex    = NULL;
SemaphoreHandle_t xParamMutex   = NULL;

volatile uint8_t  g_active_sensor = 0u;

BaseType_t app_ipc_init(void)
{
    xSensorQueue  = xQueueCreate(Q_SENSOR_LEN,  sizeof(hcsr04_event_t));
    xDisplayQueue = xQueueCreate(Q_DISPLAY_LEN, sizeof(disp_msg_t));
    xUartRxQueue  = xQueueCreate(Q_UART_RX_LEN, sizeof(uint8_t));
    xUartMutex    = xSemaphoreCreateMutex();
    xParamMutex   = xSemaphoreCreateMutex();

    if ((xSensorQueue == NULL) || (xDisplayQueue == NULL) || (xUartRxQueue == NULL) ||
        (xUartMutex == NULL) || (xParamMutex == NULL)) {
        return pdFAIL;
    }

    /* Ten hien thi trong cua so RTOS-aware khi debug */
    vQueueAddToRegistry(xSensorQueue,  "qSensor");
    vQueueAddToRegistry(xDisplayQueue, "qDisplay");
    vQueueAddToRegistry(xUartRxQueue,  "qUartRx");
    vQueueAddToRegistry(xUartMutex,    "mtxUart");
    vQueueAddToRegistry(xParamMutex,   "mtxParam");
    return pdPASS;
}

/* Bao cho task_display biet 1 thong so vua thay doi */
BaseType_t app_ipc_post_param(uint8_t param_id, uint32_t value)
{
    disp_msg_t msg;

    memset(&msg, 0, sizeof(msg));
    msg.type     = (uint8_t)DISP_PARAM;
    msg.param_id = param_id;
    msg.value    = value;
    return xQueueSend(xDisplayQueue, &msg, pdMS_TO_TICKS(20));
}
