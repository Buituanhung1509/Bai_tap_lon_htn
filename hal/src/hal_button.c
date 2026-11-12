/**
 * @file    hal_button.c
 * @brief   Driver nut nhan PB0.
 */
#include "hal_button.h"
#include "bsp_pins.h"

static volatile hal_button_cb_t s_cb;

void hal_button_set_callback(hal_button_cb_t cb)
{
    s_cb = cb;
}

uint8_t hal_button_is_pressed(void)
{
    return (HAL_GPIO_ReadPin(BSP_BTN_PORT, BSP_BTN_PIN) == BSP_BTN_ACTIVE_LEVEL) ? 1u : 0u;
}

/* Goi tu HAL_GPIO_EXTI_Callback (hal_exti.c) khi co canh xuong tren PB0 */
void hal_button_isr(BaseType_t *pxHigherPriorityTaskWoken)
{
    hal_button_cb_t cb = s_cb;
    if (cb != NULL) {
        cb(pxHigherPriorityTaskWoken);
    }
}
