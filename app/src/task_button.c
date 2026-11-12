/**
 * @file    task_button.c
 * @brief   Nut nhan PB0 chinh nguong toc do VMAX.
 *          - Nhan ngan          : VMAX += BTN_VMAX_STEP, vuot BTN_VMAX_HIGH thi ve BTN_VMAX_LOW
 *          - Giu >= BTN_LONG_MS : VMAX ve mac dinh
 *          ISR chi danh thuc task bang task notification; chong rung lam o day.
 */
#include "task_button.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "app_param.h"
#include "hal_button.h"
#include "hal_uart.h"

static TaskHandle_t xButtonTaskHandle = NULL;

static void prvButtonIsrCb(BaseType_t *pxWoken)
{
    if (xButtonTaskHandle != NULL) {
        vTaskNotifyGiveFromISR(xButtonTaskHandle, pxWoken);
    }
}

static void prvApplyVmax(uint32_t vmax, const char *reason)
{
    if (app_param_set(PARAM_VMAX, vmax) == PARAM_OK) {
        (void)hal_uart_printf("[BTN] %s -> VMAX=%lu cm/s\r\n", reason, (unsigned long)vmax);
        (void)app_ipc_post_param((uint8_t)PARAM_VMAX, vmax);
    }
}

static void prvOnShortPress(void)
{
    uint32_t v = app_param_get(PARAM_VMAX) + BTN_VMAX_STEP;
    if (v > BTN_VMAX_HIGH) {
        v = BTN_VMAX_LOW;
    }
    prvApplyVmax(v, "Nhan ngan");
}

static void prvOnLongPress(void)
{
    prvApplyVmax(PARAM_VMAX_DEF, "Giu lau (mac dinh)");
}

static void prvButtonTask(void *pvParameters)
{
    (void)pvParameters;

    for (;;) {
        TickType_t t0;
        uint8_t    long_done = 0u;

        (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);     /* cho ngat canh xuong */

        vTaskDelay(pdMS_TO_TICKS(BTN_DEBOUNCE_MS));         /* chong rung khi nhan */
        if (!hal_button_is_pressed()) {
            continue;                                       /* nhieu, bo qua */
        }

        t0 = xTaskGetTickCount();
        while (hal_button_is_pressed()) {
            if (!long_done && ((xTaskGetTickCount() - t0) >= pdMS_TO_TICKS(BTN_LONG_MS))) {
                long_done = 1u;
                prvOnLongPress();                           /* phan hoi ngay khi du 1.5 s */
            }
            vTaskDelay(pdMS_TO_TICKS(BTN_POLL_MS));
        }

        if (!long_done) {
            prvOnShortPress();
        }

        vTaskDelay(pdMS_TO_TICKS(BTN_DEBOUNCE_MS));         /* chong rung khi nha */
        (void)ulTaskNotifyTake(pdTRUE, 0);                  /* bo cac ngat do rung sinh ra */
    }
}

BaseType_t task_button_create(void)
{
    BaseType_t ret = xTaskCreate(prvButtonTask, "BUTTON", TASK_BUTTON_STACK, NULL,
                                 TASK_BUTTON_PRIO, &xButtonTaskHandle);
    if (ret == pdPASS) {
        hal_button_set_callback(prvButtonIsrCb);
    }
    return ret;
}

TaskHandle_t task_button_handle(void)
{
    return xButtonTaskHandle;
}
