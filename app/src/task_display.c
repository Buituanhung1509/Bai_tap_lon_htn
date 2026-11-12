/**
 * @file    task_display.c
 * @brief   Task so huu LCD + den/coi. Nhan disp_msg_t tu xDisplayQueue.
 *          Khong dung osDelay chan cung: thoi gian giu man hinh duoc tinh bang
 *          timeout cua xQueueReceive, nen van nhan thong diep moi trong luc giu.
 */
#include "task_display.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "app_param.h"
#include "hal_lcd.h"
#include "hal_alert.h"
#include "hal_uart.h"
#include <stdio.h>

static TaskHandle_t xDisplayTaskHandle = NULL;

static const char *prvDir(uint8_t direction)
{
    return (direction == 2u) ? "2>1" : "1>2";
}

static void prvShowReady(void)
{
    char line[17];

    hal_alert_set(ALERT_SAFE);
    hal_lcd_print_line(0u, "TRAM DO TOC DO");
    (void)snprintf(line, sizeof(line), "READY VMAX=%lu", (unsigned long)app_param_get(PARAM_VMAX));
    hal_lcd_print_line(1u, line);
}

static void prvShowWaiting(uint8_t direction)
{
    char line[17];

    hal_alert_set(ALERT_WAITING);
    (void)snprintf(line, sizeof(line), "DANG DO...  %s", prvDir(direction));
    hal_lcd_print_line(0u, line);
    (void)snprintf(line, sizeof(line), "CHO MOC %u", (unsigned)((direction == 1u) ? 2u : 1u));
    hal_lcd_print_line(1u, line);
}

static void prvShowResult(const disp_msg_t *m)
{
    char     line[17];
    uint32_t v = (m->speed_x10 > 9999u) ? 9999u : m->speed_x10;   /* toi da 999.9 */

    (void)snprintf(line, sizeof(line), "V=%lu.%lu cm/s %s",
                   (unsigned long)(v / 10u), (unsigned long)(v % 10u), prvDir(m->direction));
    hal_lcd_print_line(0u, line);

    if (m->is_violation) {
        (void)snprintf(line, sizeof(line), "VI PHAM! (>%lu)", (unsigned long)m->vmax);
        hal_alert_set(ALERT_VIOLATION);
    } else {
        (void)snprintf(line, sizeof(line), "AN TOAN (<=%lu)", (unsigned long)m->vmax);
        hal_alert_set(ALERT_SAFE);
    }
    hal_lcd_print_line(1u, line);

    (void)hal_uart_printf("[LOG] Toc do: %lu.%lu cm/s (%s), VMAX=%lu -> %s\r\n",
                          (unsigned long)(m->speed_x10 / 10u), (unsigned long)(m->speed_x10 % 10u),
                          prvDir(m->direction), (unsigned long)m->vmax,
                          m->is_violation ? "VI PHAM!" : "AN TOAN");

    if (app_param_get(PARAM_LOG) != 0u) {
        (void)hal_uart_printf("[DBG] dt=%lu us, d1=%u.%u cm, d2=%u.%u cm\r\n",
                              (unsigned long)m->dt_us,
                              (unsigned)(m->d_first_mm / 10u),  (unsigned)(m->d_first_mm % 10u),
                              (unsigned)(m->d_second_mm / 10u), (unsigned)(m->d_second_mm % 10u));
    }
}

static void prvShowTimeout(const disp_msg_t *m)
{
    char line[17];

    hal_alert_set(ALERT_TIMEOUT);
    hal_lcd_print_line(0u, "THOI GIAN CHO");
    (void)snprintf(line, sizeof(line), "TIMEOUT %lu ms", (unsigned long)m->value);
    hal_lcd_print_line(1u, line);

    (void)hal_uart_printf("[LOG] Qua moc %u nhung TIMEOUT %lu ms!\r\n",
                          (unsigned)m->direction, (unsigned long)m->value);
}

static void prvShowParam(const disp_msg_t *m)
{
    char line[17];

    hal_lcd_print_line(0u, "CAI DAT THONG SO");
    if (m->param_id == DISP_PARAM_ALL) {
        hal_lcd_print_line(1u, "VE MAC DINH");
    } else {
        (void)snprintf(line, sizeof(line), "%s=%lu %s",
                       app_param_name((param_id_t)m->param_id), (unsigned long)m->value,
                       app_param_unit((param_id_t)m->param_id));
        hal_lcd_print_line(1u, line);
    }

    /* Bip ngan xac nhan (giu nguyen trang thai coi neu dang bao vi pham) */
    if (hal_alert_get() != ALERT_VIOLATION) {
        hal_alert_buzzer(1u);
        vTaskDelay(pdMS_TO_TICKS(DISP_BEEP_MS));
        hal_alert_set(hal_alert_get());
    }
}

static void prvDisplayTask(void *pvParameters)
{
    disp_msg_t msg;
    TickType_t hold_end  = 0u;
    uint8_t    holding   = 0u;
    uint8_t    measuring = 0u;
    uint8_t    meas_dir  = 0u;

    (void)pvParameters;

    prvShowReady();

    for (;;) {
        TickType_t wait = portMAX_DELAY;

        if (holding) {
            TickType_t now  = xTaskGetTickCount();
            TickType_t left = hold_end - now;
            /* hold_end da qua -> left "am" (rat lon khi unsigned) -> doi 0 */
            wait = (left <= pdMS_TO_TICKS(DISP_RESULT_HOLD_MS + DISP_PARAM_HOLD_MS)) ? left : 0u;
        }

        if (xQueueReceive(xDisplayQueue, &msg, wait) == pdTRUE) {
            switch ((disp_type_t)msg.type) {
                case DISP_WAITING:
                    measuring = 1u;
                    meas_dir  = msg.direction;
                    holding   = 0u;          /* xe moi -> uu tien hien thi trang thai do */
                    prvShowWaiting(meas_dir);
                    break;

                case DISP_RESULT:
                    measuring = 0u;
                    prvShowResult(&msg);
                    hold_end = xTaskGetTickCount() + pdMS_TO_TICKS(DISP_RESULT_HOLD_MS);
                    holding  = 1u;
                    break;

                case DISP_TIMEOUT:
                    measuring = 0u;
                    prvShowTimeout(&msg);
                    hold_end = xTaskGetTickCount() + pdMS_TO_TICKS(DISP_RESULT_HOLD_MS);
                    holding  = 1u;
                    break;

                case DISP_PARAM:
                    prvShowParam(&msg);
                    hold_end = xTaskGetTickCount() + pdMS_TO_TICKS(DISP_PARAM_HOLD_MS);
                    holding  = 1u;
                    break;

                default:
                    break;
            }
        } else if (holding) {
            /* Het thoi gian giu man hinh */
            holding = 0u;
            if (measuring) {
                prvShowWaiting(meas_dir);
            } else {
                prvShowReady();
            }
        } else {
            /* khong xay ra (wait = portMAX_DELAY) */
        }
    }
}

BaseType_t task_display_create(void)
{
    return xTaskCreate(prvDisplayTask, "DISPLAY", TASK_DISPLAY_STACK, NULL,
                       TASK_DISPLAY_PRIO, &xDisplayTaskHandle);
}

TaskHandle_t task_display_handle(void)
{
    return xDisplayTaskHandle;
}