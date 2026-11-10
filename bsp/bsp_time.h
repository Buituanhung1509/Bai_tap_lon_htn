/**
 * @file    bsp_time.h
 * @brief   Do thoi gian do phan giai cao bang bo dem chu ky DWT->CYCCNT (72 MHz).
 *          - Dung cho do xung Echo va moc thoi gian cam bien (sai so ~14 ns).
 *          - CYCCNT tran sau 2^32 / 72 MHz ~ 59.6 s; phep tru unsigned van dung
 *            neu khoang can do < 59 s (timeout toi da cua he thong la 10 s).
 *          - Khong phu thuoc ngat -> goi duoc trong ISR, khong bi loi tran TIM4.
 */
#ifndef BSP_TIME_H
#define BSP_TIME_H

#include "stm32f1xx_hal.h"

void     bsp_time_init(void);
uint32_t bsp_cycles_to_us(uint32_t cycles);
void     bsp_delay_us(uint32_t us);

/* Doc bo dem chu ky hien tai */
__STATIC_INLINE uint32_t bsp_cycles(void)
{
    return DWT->CYCCNT;
}

#endif /* BSP_TIME_H */
