/**
 * @file    hal_alert.c
 * @brief   Den bao va coi (xem hal_alert.h).
 */
#include "hal_alert.h"
#include "bsp_pins.h"

static TIM_HandleTypeDef *s_tim_led;
static TIM_HandleTypeDef *s_tim_buz;
static alert_state_t      s_state = ALERT_IDLE;

static void prv_leds(uint8_t green, uint8_t yellow, uint8_t red)
{
    uint32_t full = __HAL_TIM_GET_AUTORELOAD(s_tim_led);
    __HAL_TIM_SET_COMPARE(s_tim_led, BSP_LED_GREEN_CH,  green  ? full : 0u);
    __HAL_TIM_SET_COMPARE(s_tim_led, BSP_LED_YELLOW_CH, yellow ? full : 0u);
    __HAL_TIM_SET_COMPARE(s_tim_led, BSP_LED_RED_CH,    red    ? full : 0u);
}

void hal_alert_buzzer(uint8_t on)
{
    uint32_t half = (__HAL_TIM_GET_AUTORELOAD(s_tim_buz) + 1u) / 2u;  /* duty 50% */
    __HAL_TIM_SET_COMPARE(s_tim_buz, BSP_BUZZER_CH, on ? half : 0u);
}

void hal_alert_init(TIM_HandleTypeDef *htim_led, TIM_HandleTypeDef *htim_buzzer)
{
    s_tim_led = htim_led;
    s_tim_buz = htim_buzzer;

    (void)HAL_TIM_PWM_Start(s_tim_led, BSP_LED_GREEN_CH);
    (void)HAL_TIM_PWM_Start(s_tim_led, BSP_LED_YELLOW_CH);
    (void)HAL_TIM_PWM_Start(s_tim_led, BSP_LED_RED_CH);
    (void)HAL_TIM_PWM_Start(s_tim_buz, BSP_BUZZER_CH);

    hal_alert_set(ALERT_IDLE);
}

void hal_alert_set(alert_state_t state)
{
    s_state = state;
    switch (state) {
        case ALERT_IDLE:
        case ALERT_SAFE:
            prv_leds(1u, 0u, 0u);
            hal_alert_buzzer(0u);
            break;
        case ALERT_WAITING:
            prv_leds(0u, 1u, 0u);
            hal_alert_buzzer(0u);
            break;
        case ALERT_VIOLATION:
            prv_leds(0u, 0u, 1u);
            hal_alert_buzzer(1u);
            break;
        case ALERT_TIMEOUT:
        default:
            prv_leds(0u, 0u, 0u);
            hal_alert_buzzer(0u);
            break;
    }
}

alert_state_t hal_alert_get(void)
{
    return s_state;
}
/*tedadadadast*/