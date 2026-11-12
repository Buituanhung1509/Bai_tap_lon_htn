/**
 * @file    hal_uart.c
 * @brief   Driver UART 2 chieu (xem mo ta trong hal_uart.h).
 */
#include "hal_uart.h"
#include "task.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define UART_TX_BUF_SIZE      128u
#define UART_LOCK_TIMEOUT_MS  300u
#define UART_TX_TIMEOUT_MS    250u   /* 128 byte @ 9600 baud ~ 133 ms */

static UART_HandleTypeDef *s_huart;
static SemaphoreHandle_t   s_lock;
static hal_uart_rx_cb_t    s_rx_cb;
static TaskHandle_t        s_tx_waiter;
static char                s_txbuf[UART_TX_BUF_SIZE];  /* chi truy cap khi dang giu mutex */

static int prv_scheduler_running(void)
{
    return (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) ? 1 : 0;
}

static int prv_lock(void)
{
    if ((s_lock != NULL) && prv_scheduler_running()) {
        return (xSemaphoreTake(s_lock, pdMS_TO_TICKS(UART_LOCK_TIMEOUT_MS)) == pdTRUE) ? 1 : 0;
    }
    return 1;
}

static void prv_unlock(void)
{
    if ((s_lock != NULL) && prv_scheduler_running()) {
        (void)xSemaphoreGive(s_lock);
    }
}

/* Gui du lieu, KHONG lay mutex (ben goi da giu mutex hoac chua co scheduler) */
static HAL_StatusTypeDef prv_send(const uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef st;

    if (!prv_scheduler_running()) {
        return HAL_UART_Transmit(s_huart, (uint8_t *)data, len, UART_TX_TIMEOUT_MS);
    }

    s_tx_waiter = xTaskGetCurrentTaskHandle();
    (void)ulTaskNotifyTake(pdTRUE, 0);                 /* xoa thong bao cu neu con */
    st = HAL_UART_Transmit_IT(s_huart, (uint8_t *)data, len);
    if (st == HAL_OK) {
        if (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(UART_TX_TIMEOUT_MS)) == 0u) {
            (void)HAL_UART_AbortTransmit(s_huart);     /* qua thoi gian -> huy */
            st = HAL_TIMEOUT;
        }
    }
    s_tx_waiter = NULL;
    return st;
}

void hal_uart_init(UART_HandleTypeDef *huart)
{
    s_huart = huart;
}

void hal_uart_set_lock(SemaphoreHandle_t mutex)
{
    s_lock = mutex;
}

void hal_uart_start_rx(hal_uart_rx_cb_t cb)
{
    s_rx_cb = cb;
    /* Doc bo SR, DR de xoa co RXNE/ORE con sot lai truoc khi bat ngat */
    (void)s_huart->Instance->SR;
    (void)s_huart->Instance->DR;
    __HAL_UART_ENABLE_IT(s_huart, UART_IT_RXNE);
}

HAL_StatusTypeDef hal_uart_write(const uint8_t *data, uint16_t len)
{
    HAL_StatusTypeDef st;

    if ((s_huart == NULL) || (data == NULL) || (len == 0u)) {
        return HAL_ERROR;
    }
    if (!prv_lock()) {
        return HAL_BUSY;
    }
    st = prv_send(data, len);
    prv_unlock();
    return st;
}

int hal_uart_printf(const char *fmt, ...)
{
    va_list ap;
    int n;

    if (s_huart == NULL) {
        return -1;
    }
    if (!prv_lock()) {
        return -1;
    }

    va_start(ap, fmt);
    n = vsnprintf(s_txbuf, sizeof(s_txbuf), fmt, ap);
    va_end(ap);

    if (n > 0) {
        if (n >= (int)sizeof(s_txbuf)) {
            n = (int)sizeof(s_txbuf) - 1;   /* cat bot neu qua dai */
        }
        (void)prv_send((const uint8_t *)s_txbuf, (uint16_t)n);
    }
    prv_unlock();
    return n;
}

/* Gui truc tiep kieu polling, bo qua mutex - CHI dung khi he thong da loi (hook) */
void hal_uart_write_panic(const char *str)
{
    if ((s_huart != NULL) && (str != NULL)) {
        (void)HAL_UART_AbortTransmit(s_huart);
        (void)HAL_UART_Transmit(s_huart, (uint8_t *)str, (uint16_t)strlen(str), 100u);
    }
}

/* Goi trong USART1_IRQHandler, truoc HAL_UART_IRQHandler.
 * Tu doc byte nhan duoc (HAL khong o che do nhan nen se khong dung vao RX). */
void hal_uart_irq_handler(void)
{
    uint32_t sr;
    uint8_t  byte;
    BaseType_t woken = pdFALSE;

    if (s_huart == NULL) {
        return;
    }

    sr = s_huart->Instance->SR;
    if ((sr & (USART_SR_RXNE | USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE)) != 0u) {
        byte = (uint8_t)(s_huart->Instance->DR & 0xFFu);  /* doc SR roi DR -> xoa RXNE + co loi */
        if (((sr & USART_SR_RXNE) != 0u) && ((sr & (USART_SR_FE | USART_SR_PE)) == 0u) &&
            (s_rx_cb != NULL)) {
            s_rx_cb(byte, &woken);
        }
    }
    portYIELD_FROM_ISR(woken);
}

/* ---------------- Callback cua HAL ---------------- */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    BaseType_t woken = pdFALSE;

    if ((huart == s_huart) && (s_tx_waiter != NULL)) {
        vTaskNotifyGiveFromISR(s_tx_waiter, &woken);
    }
    portYIELD_FROM_ISR(woken);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    /* HAL tat RXNEIE khi gap loi overrun -> bat lai de khong mat chieu nhan */
    if ((huart == s_huart) && (s_rx_cb != NULL)) {
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
    }
}
