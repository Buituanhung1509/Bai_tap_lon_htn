/**
 * @file    hal_lcd.h
 * @brief   Driver LCD 16x2 (HD44780) qua module I2C PCF8574, che do 4-bit.
 *          Chi task_display duoc goi sau khi scheduler chay (1 task so huu LCD,
 *          nen khong can mutex). Ham tre tu chon vTaskDelay/HAL_Delay.
 */
#ifndef HAL_LCD_H
#define HAL_LCD_H

#include "stm32f1xx_hal.h"

void hal_lcd_init(I2C_HandleTypeDef *hi2c);
void hal_lcd_clear(void);
void hal_lcd_set_cursor(uint8_t row, uint8_t col);
void hal_lcd_print(const char *str);
void hal_lcd_print_line(uint8_t row, const char *str);

#endif /* HAL_LCD_H */
