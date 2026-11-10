/**
 * @file    hal_button.h
 * @brief   Driver nut nhan PB0 (EXTI0). ISR chi bao hieu qua callback,
 *          chong rung (debounce) va phan biet nhan ngan/giu do task_button lam.
 */
#ifndef HAL_BUTTON_H
#define HAL_BUTTON_H

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"

typedef void (*hal_button_cb_t)(BaseType_t *pxHigherPriorityTaskWoken);

void    hal_button_set_callback(hal_button_cb_t cb);
uint8_t hal_button_is_pressed(void);
void    hal_button_isr(BaseType_t *pxHigherPriorityTaskWoken);

#endif /* HAL_BUTTON_H */
