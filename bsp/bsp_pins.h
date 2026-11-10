/**
 * @file    bsp_pins.h
 * @brief   Board support: bang anh xa chan cua board (Blue Pill STM32F103C8T6).
 *          Moi thay doi dau noi phan cung chi sua O DAY.
 *          Cac chan nay phai trung voi cau hinh trong BAI_TAP_LON.ioc.
 */
#ifndef BSP_PINS_H
#define BSP_PINS_H

#include "stm32f1xx_hal.h"

/* ---------------- HC-SR04 ---------------- */
#define BSP_TRIG1_PORT        GPIOA
#define BSP_TRIG1_PIN         GPIO_PIN_3      /* Trig cam bien 1 (moc 1) */
#define BSP_TRIG2_PORT        GPIOA
#define BSP_TRIG2_PIN         GPIO_PIN_4      /* Trig cam bien 2 (moc 2) */

#define BSP_ECHO1_PORT        GPIOB
#define BSP_ECHO1_PIN         GPIO_PIN_12     /* Echo 1 - EXTI12, 2 canh */
#define BSP_ECHO2_PORT        GPIOB
#define BSP_ECHO2_PIN         GPIO_PIN_13     /* Echo 2 - EXTI13, 2 canh */

/* ---------------- Nut nhan ---------------- */
/* Nut noi PB0 xuong GND, dung dien tro keo len noi -> nhan = muc 0 */
#define BSP_BTN_PORT          GPIOB
#define BSP_BTN_PIN           GPIO_PIN_0      /* EXTI0, canh xuong */
#define BSP_BTN_ACTIVE_LEVEL  GPIO_PIN_RESET

/* ---------------- LCD 16x2 qua PCF8574 (I2C1: PB6 SCL, PB7 SDA) ---------------- */
#define BSP_LCD_I2C_ADDR      (0x27u << 1)    /* dia chi 7-bit 0x27, dich trai cho HAL */
#define BSP_LCD_COLS          16u

/* ---------------- Canh bao ---------------- */
/* TIM2 PWM: CH1 = PA0 (xanh), CH2 = PA1 (vang), CH3 = PA2 (do) */
#define BSP_LED_GREEN_CH      TIM_CHANNEL_1
#define BSP_LED_YELLOW_CH     TIM_CHANNEL_2
#define BSP_LED_RED_CH        TIM_CHANNEL_3
/* TIM3 PWM: CH1 = PA6 (coi) */
#define BSP_BUZZER_CH         TIM_CHANNEL_1

/* ---------------- UART log/lenh: USART1 PA9 TX, PA10 RX ---------------- */

#endif /* BSP_PINS_H */
