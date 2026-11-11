/**
 * @file    task_calc.c
 * @brief   Tinh toc do tu 2 moc cam bien.
 *          v (cm/s) = S (mm) / 10 / (dt (us) / 1e6)  ->  v*10 = S(mm) * 1e6 / dt(us)
 *          Moc thoi gian lay bang DWT trong ISR -> dt chinh xac toi micro giay.
 */
#include "task_calc.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "app_param.h"
#include "bsp_time.h"
#include "hal_hcsr04.h"
#include "hal_uart.h"
#include <string.h>

static TaskHandle_t xCalcTaskHandle = NULL;

/* Callback chay trong ngat Echo: day su kien vao queue */
static void prvSensorIsrCb(const hcsr04_event_t *evt, BaseType_t *pxWoken)
{
    if (xSensorQueue != NULL) {
        (void)xQueueSendFromISR(xSensorQueue, evt, pxWoken);
    }
}

static void prvPostDisplay(const disp_msg_t *msg)
{
    (void)xQueueSend(xDisplayQueue, msg, pdMS_TO_TICKS(20));
}

static void prvCalcTask(void *pvParameters)
{
    hcsr04_event_t ev;
    disp_msg_t     msg;
    uint32_t       t1_cycles = 0u;
    TickType_t     t1_tick   = 0u;
    uint16_t       d_first   = 0u;

    (void)pvParameters;

    for (;;) {
        if (xQueueReceive(xSensorQueue, &ev, pdMS_TO_TICKS(CALC_POLL_MS)) == pdTRUE) {
            uint8_t first = g_active_sensor;

            if (first == 0u) {
                /* ---- MOC DAU TIEN ---- */
                t1_cycles = ev.t_cycles;
                t1_tick   = xTaskGetTickCount();
                d_first   = ev.dist_mm;
                g_active_sensor = ev.id;

                memset(&msg, 0, sizeof(msg));
                msg.type      = (uint8_t)DISP_WAITING;
                msg.direction = ev.id;
                prvPostDisplay(&msg);

                (void)hal_uart_printf("[LOG] Qua moc %u -> dang cho moc %u...\r\n",
                                      (unsigned)ev.id,
                                      (unsigned)((ev.id == HCSR04_ID_1) ? HCSR04_ID_2 : HCSR04_ID_1));
            } else if (ev.id != first) {
                /* ---- MOC THU HAI ---- */
                uint32_t dt_us = bsp_cycles_to_us(ev.t_cycles - t1_cycles);

                if (dt_us >= CALC_MIN_DT_US) {
                    uint32_t dist_mm = app_param_get(PARAM_DIST);   /* <= 500 mm */
                    uint32_t vmax    = app_param_get(PARAM_VMAX);

                    memset(&msg, 0, sizeof(msg));
                    msg.type         = (uint8_t)DISP_RESULT;
                    msg.direction    = first;
                    /* 500 * 1e6 = 5e8 < 2^32 -> khong tran */
                    msg.speed_x10    = (dist_mm * 1000000u) / dt_us;
                    msg.vmax         = vmax;
                    msg.is_violation = (msg.speed_x10 > (vmax * 10u)) ? 1u : 0u;
                    msg.dt_us        = dt_us;
                    msg.d_first_mm   = d_first;
                    msg.d_second_mm  = ev.dist_mm;

                    g_active_sensor = 0u;    /* do xong -> quay lai quet ca 2 cam bien */
                    prvPostDisplay(&msg);
                }
            } else {
                /* cung cam bien voi moc dau -> bo qua */
            }
        }

        /* ---- KIEM TRA TIMEOUT ---- */
        if ((g_active_sensor != 0u) &&
            ((xTaskGetTickCount() - t1_tick) > pdMS_TO_TICKS(app_param_get(PARAM_TOUT)))) {
            memset(&msg, 0, sizeof(msg));
            msg.type      = (uint8_t)DISP_TIMEOUT;
            msg.direction = g_active_sensor;
            msg.value     = app_param_get(PARAM_TOUT);
            g_active_sensor = 0u;
            prvPostDisplay(&msg);
        }
    }
}

BaseType_t task_calc_create(void)
{
    BaseType_t ret = xTaskCreate(prvCalcTask, "CALC", TASK_CALC_STACK, NULL,
                                 TASK_CALC_PRIO, &xCalcTaskHandle);
    if (ret == pdPASS) {
        hal_hcsr04_set_callback(prvSensorIsrCb);
    }
    return ret;
}

TaskHandle_t task_calc_handle(void)
{
    return xCalcTaskHandle;
}
