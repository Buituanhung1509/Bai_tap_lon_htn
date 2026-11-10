/**
 * @file    app_main.c
 * @brief   Khoi tao ung dung: driver -> IPC -> task (xem app_main.h).
 */
#include "app_main.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "app_param.h"
#include "bsp_time.h"
#include "hal_uart.h"
#include "hal_lcd.h"
#include "hal_alert.h"
#include "hal_hcsr04.h"
#include "task_sensor.h"
#include "task_calc.h"
#include "task_display.h"
#include "task_comm.h"
#include "task_button.h"
#include "main.h"

#if I2C_SCAN_ENABLE
static void prvI2cScan(I2C_HandleTypeDef *hi2c)
{
    uint8_t a;
    uint8_t found = 0u;

    (void)hal_uart_printf("[I2C] Dang quet...\r\n");
    for (a = 0x08u; a < 0x78u; a++) {
        if (HAL_I2C_IsDeviceReady(hi2c, (uint16_t)(a << 1), 1u, 10u) == HAL_OK) {
            (void)hal_uart_printf("[I2C] Tim thay: 0x%02X\r\n", (unsigned)a);
            found++;
        }
    }
    if (found == 0u) {
        (void)hal_uart_printf("[I2C] KHONG thay thiet bi nao!\r\n");
    }
}
#endif

void app_hw_init(I2C_HandleTypeDef *hi2c, TIM_HandleTypeDef *htim_led,
                 TIM_HandleTypeDef *htim_buzzer, UART_HandleTypeDef *huart)
{
    bsp_time_init();
    hal_uart_init(huart);
    (void)hal_uart_printf("\r\n=== TRAM DO TOC DO v%s ===\r\n", APP_FW_VERSION);

#if I2C_SCAN_ENABLE
    prvI2cScan(hi2c);
#endif

    hal_hcsr04_init();
    app_param_init();            /* nap mac dinh + day nguong phat hien xuong driver */
    hal_alert_init(htim_led, htim_buzzer);
    hal_lcd_init(hi2c);
    hal_lcd_print_line(0u, "TRAM DO TOC DO");
    hal_lcd_print_line(1u, "FW v" APP_FW_VERSION " BOOT");
}

void app_rtos_init(void)
{
    if (app_ipc_init() != pdPASS) {
        app_fatal("IPC", "khong du heap tao queue/mutex");
    }
    hal_uart_set_lock(xUartMutex);

    if (task_sensor_create()  != pdPASS) { app_fatal("TASK", "SENSOR");  }
    if (task_calc_create()    != pdPASS) { app_fatal("TASK", "CALC");    }
    if (task_display_create() != pdPASS) { app_fatal("TASK", "DISPLAY"); }
    if (task_comm_create()    != pdPASS) { app_fatal("TASK", "COMM");    }
    if (task_button_create()  != pdPASS) { app_fatal("TASK", "BUTTON");  }

    (void)hal_uart_printf("[RTOS] Tao task OK, heap con %lu byte\r\n",
                          (unsigned long)xPortGetFreeHeapSize());
}

/* Loi nghiem trong: bao qua UART (polling) + den do, roi dung han */
void app_fatal(const char *reason, const char *detail)
{
    taskDISABLE_INTERRUPTS();
    hal_alert_set(ALERT_VIOLATION);
    hal_uart_write_panic("\r\n[FATAL] ");
    hal_uart_write_panic(reason);
    hal_uart_write_panic(": ");
    hal_uart_write_panic(detail);
    hal_uart_write_panic("\r\n");
    Error_Handler();
}
