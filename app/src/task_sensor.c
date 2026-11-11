/**
 * @file    task_sensor.c
 * @brief   Phat xung Trig cho HC-SR04.
 *          - Chua co moc nao: phat luan phien ca 2 cam bien -> do duoc 2 chieu.
 *          - Da qua 1 moc  : chi phat cam bien con lai (tang tan so lay mau).
 *          Ket qua do nam trong ngat Echo (hal_hcsr04), task nay chi tao nhip.
 */
#include "task_sensor.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "hal_hcsr04.h"

static TaskHandle_t xSensorTaskHandle = NULL;

static void prvSensorTask(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(SENSOR_PERIOD_MS);
    uint8_t turn = HCSR04_ID_1;

    (void)pvParameters;

    for (;;) {
        uint8_t active = g_active_sensor;

        if (active == 0u) {
            hal_hcsr04_trigger(turn);
            turn = (turn == HCSR04_ID_1) ? HCSR04_ID_2 : HCSR04_ID_1;
        } else {
            hal_hcsr04_trigger((active == HCSR04_ID_1) ? HCSR04_ID_2 : HCSR04_ID_1);
        }

        vTaskDelayUntil(&xLastWakeTime, xPeriod);   /* doi song doi am suy hao het */
    }
}

BaseType_t task_sensor_create(void)
{
    return xTaskCreate(prvSensorTask, "SENSOR", TASK_SENSOR_STACK, NULL,
                       TASK_SENSOR_PRIO, &xSensorTaskHandle);
}

TaskHandle_t task_sensor_handle(void)
{
    return xSensorTaskHandle;
}
