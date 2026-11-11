/**
 * @file    hal_exti.c
 * @brief   Bo phan phoi ngat EXTI: HAL chi co 1 ham HAL_GPIO_EXTI_Callback cho
 *          moi chan, file nay chuyen tung chan toi dung driver.
 *          Uu tien ngat EXTI0 va EXTI15_10 phai >= 5 vi driver goi API ...FromISR.
 */
#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_pins.h"
#include "hal_hcsr04.h"
#include "hal_button.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t woken = pdFALSE;

    if (GPIO_Pin == BSP_ECHO1_PIN) {
        hal_hcsr04_echo_isr(HCSR04_ID_1, &woken);
    } else if (GPIO_Pin == BSP_ECHO2_PIN) {
        hal_hcsr04_echo_isr(HCSR04_ID_2, &woken);
    } else if (GPIO_Pin == BSP_BTN_PIN) {
        hal_button_isr(&woken);
    } else {
        /* chan khong dung */
    }

    portYIELD_FROM_ISR(woken);
}
