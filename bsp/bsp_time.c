/**
 * @file    bsp_time.c
 * @brief   Bo dem chu ky DWT dung lam dong ho microgiay.
 */
#include "bsp_time.h"

static uint32_t s_cyc_per_us = 72u;

void bsp_time_init(void)
{
    s_cyc_per_us = SystemCoreClock / 1000000u;
    if (s_cyc_per_us == 0u) {
        s_cyc_per_us = 1u;
    }

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  /* bat khoi trace (DWT) */
    DWT->CYCCNT = 0u;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;            /* bat bo dem chu ky   */
}

uint32_t bsp_cycles_to_us(uint32_t cycles)
{
    return cycles / s_cyc_per_us;
}

/* Tre ngan (vai chuc us) kieu busy-wait, chi dung cho xung Trig / xung Enable LCD */
void bsp_delay_us(uint32_t us)
{
    uint32_t start  = DWT->CYCCNT;
    uint32_t target = us * s_cyc_per_us;
    while ((DWT->CYCCNT - start) < target) {
        /* cho */
    }
}
