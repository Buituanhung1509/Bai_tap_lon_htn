/**
 * @file    app_main.h
 * @brief   Diem vao cua ung dung, goi tu main.c (trong vung USER CODE):
 *            1. app_hw_init()   - USER CODE 2        : khoi tao driver, chua co RTOS
 *            2. app_rtos_init() - USER CODE RTOS_THREADS: tao IPC + tat ca task
 *          Sau do main.c goi osKernelStart().
 *
 *  Luu y: phai khoi tao xong phan cung (co dung HAL_Delay) TRUOC khi tao bat ky
 *  doi tuong FreeRTOS nao, vi FreeRTOS chan ngat uu tien thap (TIM4 tick cua HAL)
 *  tu luc tao doi tuong dau tien den khi scheduler chay -> HAL_Delay se treo.
 */
#ifndef APP_MAIN_H
#define APP_MAIN_H

#include "stm32f1xx_hal.h"

void app_hw_init(I2C_HandleTypeDef *hi2c, TIM_HandleTypeDef *htim_led,
                 TIM_HandleTypeDef *htim_buzzer, UART_HandleTypeDef *huart);
void app_rtos_init(void);
void app_fatal(const char *reason, const char *detail);

#endif /* APP_MAIN_H */
