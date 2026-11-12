/**
 * @file    hal_lcd.c
 * @brief   Driver LCD 16x2 qua PCF8574 (xem hal_lcd.h).
 *
 *  Bit PCF8574: P0=RS, P1=RW, P2=EN, P3=den nen, P4..P7 = D4..D7
 */
#include "hal_lcd.h"
#include "bsp_pins.h"
#include "bsp_time.h"
#include "FreeRTOS.h"
#include "task.h"

#define LCD_BIT_RS        0x01u
#define LCD_BIT_EN        0x04u
#define LCD_BIT_BL        0x08u
#define LCD_I2C_TIMEOUT   10u

static I2C_HandleTypeDef *s_hi2c;

/* Truoc khi scheduler chay dung HAL_Delay, sau do dung vTaskDelay (quy chuan RTOS) */
static void prv_delay_ms(uint32_t ms)
{
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        TickType_t t = pdMS_TO_TICKS(ms);
        vTaskDelay((t == 0u) ? 1u : t);
    } else {
        HAL_Delay(ms);
    }
}

static void prv_write_nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (uint8_t)((nibble & 0xF0u) | rs | LCD_BIT_BL);
    uint8_t buf[2];

    buf[0] = (uint8_t)(data | LCD_BIT_EN);          /* EN = 1 */
    buf[1] = (uint8_t)(data & (uint8_t)~LCD_BIT_EN); /* EN = 0 -> suon xuong chot du lieu */
    (void)HAL_I2C_Master_Transmit(s_hi2c, BSP_LCD_I2C_ADDR, buf, 2u, LCD_I2C_TIMEOUT);
    bsp_delay_us(50u);
}

static void prv_send(uint8_t value, uint8_t rs)
{
    prv_write_nibble((uint8_t)(value & 0xF0u), rs);
    prv_write_nibble((uint8_t)((value << 4) & 0xF0u), rs);
    bsp_delay_us(50u);
}

void hal_lcd_init(I2C_HandleTypeDef *hi2c)
{
    s_hi2c = hi2c;

    prv_delay_ms(50u);                 /* cho nguon LCD on dinh */

    /* Chuoi reset chuan HD44780 vao che do 4-bit */
    prv_write_nibble(0x30u, 0u); prv_delay_ms(5u);
    prv_write_nibble(0x30u, 0u); prv_delay_ms(5u);
    prv_write_nibble(0x30u, 0u); prv_delay_ms(1u);
    prv_write_nibble(0x20u, 0u); prv_delay_ms(5u);

    prv_send(0x28u, 0u);               /* 4-bit, 2 dong, font 5x8 */
    prv_send(0x0Cu, 0u);               /* bat hien thi, tat con tro */
    prv_send(0x06u, 0u);               /* tu tang con tro */
    hal_lcd_clear();
}

void hal_lcd_clear(void)
{
    prv_send(0x01u, 0u);
    prv_delay_ms(2u);                  /* lenh xoa can >= 1.52 ms */
}

void hal_lcd_set_cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (uint8_t)(((row == 0u) ? 0x80u : 0xC0u) + col);
    prv_send(addr, 0u);
}

void hal_lcd_print(const char *str)
{
    while (*str != '\0') {
        prv_send((uint8_t)*str, LCD_BIT_RS);
        str++;
    }
}

/* Ghi ca dong 16 ky tu (thieu thi them dau cach) -> khong can xoa man hinh, khong nhap nhay */
void hal_lcd_print_line(uint8_t row, const char *str)
{
    uint8_t i;

    hal_lcd_set_cursor(row, 0u);
    for (i = 0u; i < BSP_LCD_COLS; i++) {
        char c = ' ';
        if (*str != '\0') {
            c = *str;
            str++;
        }
        prv_send((uint8_t)c, LCD_BIT_RS);
    }
}
