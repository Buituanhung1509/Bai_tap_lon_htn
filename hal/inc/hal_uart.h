/**
 * @file    hal_uart.h
 * @brief   Driver UART 2 chieu, an toan da luong (thread-safe) voi FreeRTOS.
 *
 *  TX: - Truoc khi scheduler chay: gui kieu polling (dung trong khoi tao).
 *      - Sau khi scheduler chay  : gui bang ngat (HAL_UART_Transmit_IT), task
 *        nhuong CPU trong luc cho, co mutex de cac task khong chen log vao nhau.
 *  RX: ngat RXNE tung byte, moi byte duoc dua ra qua callback (goi trong ISR,
 *      callback chi duoc dung ham ...FromISR).
 *
 *  Bat buoc: goi hal_uart_irq_handler() trong USART1_IRQHandler (stm32f1xx_it.c),
 *  TRUOC HAL_UART_IRQHandler(). Uu tien ngat USART1 phai >= 5 (configMAX_SYSCALL).
 */
#ifndef HAL_UART_H
#define HAL_UART_H

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"

typedef void (*hal_uart_rx_cb_t)(uint8_t byte, BaseType_t *pxHigherPriorityTaskWoken);

void              hal_uart_init(UART_HandleTypeDef *huart);
void              hal_uart_set_lock(SemaphoreHandle_t mutex);
void              hal_uart_start_rx(hal_uart_rx_cb_t cb);
HAL_StatusTypeDef hal_uart_write(const uint8_t *data, uint16_t len);
int               hal_uart_printf(const char *fmt, ...);
void              hal_uart_write_panic(const char *str);
void              hal_uart_irq_handler(void);

#endif /* HAL_UART_H */
