/**
 * @file    hal_alert.h
 * @brief   Den bao 3 mau (TIM2 PWM) va coi (TIM3 PWM).
 *          Chi task_display dieu khien sau khi scheduler chay.
 */
#ifndef HAL_ALERT_H
#define HAL_ALERT_H

#include "stm32f1xx_hal.h"

typedef enum {
    ALERT_IDLE = 0,
    ALERT_SAFE,        /* xanh */
    ALERT_WAITING,     /* vang: da qua moc dau, dang cho moc con lai */
    ALERT_VIOLATION,   /* do + coi */
    ALERT_TIMEOUT      /* tat het */
} alert_state_t;

void          hal_alert_init(TIM_HandleTypeDef *htim_led, TIM_HandleTypeDef *htim_buzzer);
void          hal_alert_set(alert_state_t state);
alert_state_t hal_alert_get(void);
void          hal_alert_buzzer(uint8_t on);

#endif /* HAL_ALERT_H */
