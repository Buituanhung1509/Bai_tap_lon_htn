/**
 * @file    hal_hcsr04.c
 * @brief   Driver HC-SR04 (xem hal_hcsr04.h).
 */
#include "hal_hcsr04.h"
#include "bsp_pins.h"
#include "bsp_time.h"

#define HCSR04_MIN_MM       15u   /* < 1.5 cm coi la nhieu */
#define HCSR04_TRIG_US      12u   /* xung Trig >= 10 us */
#define FAR_CLEAR_COUNT     3u    /* so lan doc "xa" lien tiep de coi la vat da roi di */

static volatile hal_hcsr04_cb_t s_cb;
static volatile uint16_t        s_detect_mm = 150u;

/* index = id (1, 2); phan tu 0 khong dung */
static uint32_t s_rise_cyc[3];
static uint8_t  s_present[3];
static uint8_t  s_far_cnt[3];

void hal_hcsr04_init(void)
{
    HAL_GPIO_WritePin(BSP_TRIG1_PORT, BSP_TRIG1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BSP_TRIG2_PORT, BSP_TRIG2_PIN, GPIO_PIN_RESET);
}

void hal_hcsr04_set_callback(hal_hcsr04_cb_t cb)
{
    s_cb = cb;
}

/* Ghi 16-bit la nguyen tu tren Cortex-M3 -> ISR doc an toan khong can mutex */
void hal_hcsr04_set_detect_mm(uint16_t mm)
{
    s_detect_mm = mm;
}

void hal_hcsr04_trigger(uint8_t id)
{
    GPIO_TypeDef *port;
    uint16_t      pin;

    if (id == HCSR04_ID_1) {
        port = BSP_TRIG1_PORT; pin = BSP_TRIG1_PIN;
    } else if (id == HCSR04_ID_2) {
        port = BSP_TRIG2_PORT; pin = BSP_TRIG2_PIN;
    } else {
        return;
    }

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    bsp_delay_us(HCSR04_TRIG_US);
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
}

/* Goi tu HAL_GPIO_EXTI_Callback (hal_exti.c) o ca canh len va canh xuong cua Echo */
void hal_hcsr04_echo_isr(uint8_t id, BaseType_t *pxHigherPriorityTaskWoken)
{
    uint32_t      now = bsp_cycles();
    GPIO_TypeDef *port;
    uint16_t      pin;
    uint32_t      pulse_us;
    uint32_t      dist_mm;

    if (id == HCSR04_ID_1) {
        port = BSP_ECHO1_PORT; pin = BSP_ECHO1_PIN;
    } else if (id == HCSR04_ID_2) {
        port = BSP_ECHO2_PORT; pin = BSP_ECHO2_PIN;
    } else {
        return;
    }

    if (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_SET) {
        s_rise_cyc[id] = now;               /* canh len: bat dau do */
        return;
    }

    /* canh xuong: tinh khoang cach, d(mm) = t(us) * 10 / 58 */
    pulse_us = bsp_cycles_to_us(now - s_rise_cyc[id]);
    dist_mm  = (pulse_us * 10u) / 58u;

    if ((dist_mm > HCSR04_MIN_MM) && (dist_mm < s_detect_mm)) {
        s_far_cnt[id] = 0u;
        if (s_present[id] == 0u) {
            hal_hcsr04_cb_t cb = s_cb;
            s_present[id] = 1u;
            if (cb != NULL) {
                hcsr04_event_t evt;
                evt.id       = id;
                evt.dist_mm  = (uint16_t)dist_mm;
                evt.t_cycles = s_rise_cyc[id];
                cb(&evt, pxHigherPriorityTaskWoken);
            }
        }
    } else {
        if (s_far_cnt[id] < 255u) {
            s_far_cnt[id]++;
        }
        if (s_far_cnt[id] >= FAR_CLEAR_COUNT) {
            s_present[id] = 0u;
        }
    }
}
