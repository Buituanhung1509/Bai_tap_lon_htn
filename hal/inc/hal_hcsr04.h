/**
 * @file    hal_hcsr04.h
 * @brief   Driver 2 cam bien sieu am HC-SR04.
 *          - Trig: phat xung 12 us.
 *          - Echo: ngat EXTI 2 canh, do do rong xung bang DWT trong ISR.
 *          - Chi phat su kien khi vat MOI VAO vung do (tu "khong co" -> "co"),
 *            vat dung yen trong vung do khong sinh them moc gia.
 */
#ifndef HAL_HCSR04_H
#define HAL_HCSR04_H

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"

#define HCSR04_ID_1   1u   /* moc 1: Trig PA3, Echo PB12 */
#define HCSR04_ID_2   2u   /* moc 2: Trig PA4, Echo PB13 */

/* Su kien "co vat moi vao vung do" */
typedef struct {
    uint8_t  id;         /* HCSR04_ID_1 / HCSR04_ID_2 */
    uint16_t dist_mm;    /* khoang cach luc phat hien (mm) */
    uint32_t t_cycles;   /* thoi diem do (canh len Echo), don vi chu ky DWT */
} hcsr04_event_t;

typedef void (*hal_hcsr04_cb_t)(const hcsr04_event_t *evt, BaseType_t *pxHigherPriorityTaskWoken);

void hal_hcsr04_init(void);
void hal_hcsr04_set_callback(hal_hcsr04_cb_t cb);
void hal_hcsr04_set_detect_mm(uint16_t mm);
void hal_hcsr04_trigger(uint8_t id);
void hal_hcsr04_echo_isr(uint8_t id, BaseType_t *pxHigherPriorityTaskWoken);

#endif /* HAL_HCSR04_H */
