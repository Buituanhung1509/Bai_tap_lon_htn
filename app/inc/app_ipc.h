/**
 * @file    app_ipc.h
 * @brief   Doi tuong IPC dung chung giua cac task: queue, mutex, kieu goi tin.
 *
 *  Luong du lieu:
 *    EXTI Echo  --hcsr04_event_t--> xSensorQueue  --> task_calc
 *    task_calc  --disp_msg_t------> xDisplayQueue --> task_display
 *    USART1 RX  --uint8_t---------> xUartRxQueue  --> task_comm
 *    task_comm / task_button --(DISP_PARAM)--> xDisplayQueue
 *    Moi task ghi log qua hal_uart_printf() (bao ve boi xUartMutex)
 *    Thong so doc/ghi qua app_param (bao ve boi xParamMutex)
 */
#ifndef APP_IPC_H
#define APP_IPC_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "hal_hcsr04.h"

/* Loai thong diep gui toi task_display */
typedef enum {
    DISP_WAITING = 0,   /* da qua moc dau, dang cho moc con lai */
    DISP_RESULT,        /* co ket qua toc do */
    DISP_TIMEOUT,       /* qua moc dau nhung het thoi gian cho */
    DISP_PARAM          /* thong so vua thay doi (UART / nut nhan) */
} disp_type_t;

#define DISP_PARAM_ALL   0xFFu   /* param_id khi RESET toan bo ve mac dinh */

typedef struct {
    uint8_t  type;          /* disp_type_t */
    uint8_t  direction;     /* 1: moc1 -> moc2, 2: moc2 -> moc1 */
    uint8_t  is_violation;
    uint8_t  param_id;      /* DISP_PARAM: param_id_t hoac DISP_PARAM_ALL */
    uint32_t speed_x10;     /* toc do x10 (cm/s), vd 453 = 45.3 cm/s */
    uint32_t vmax;          /* nguong dung de so sanh luc do */
    uint32_t dt_us;         /* thoi gian giua 2 moc (us) */
    uint32_t value;         /* DISP_PARAM: gia tri moi */
    uint16_t d_first_mm;
    uint16_t d_second_mm;
} disp_msg_t;

extern QueueHandle_t     xSensorQueue;
extern QueueHandle_t     xDisplayQueue;
extern QueueHandle_t     xUartRxQueue;
extern SemaphoreHandle_t xUartMutex;
extern SemaphoreHandle_t xParamMutex;

/* Cam bien dang cho: 0 = chua co moc nao, 1/2 = cam bien da kich hoat truoc.
 * task_calc ghi, task_sensor doc (bien 8-bit, truy cap nguyen tu). */
extern volatile uint8_t  g_active_sensor;

BaseType_t app_ipc_init(void);
BaseType_t app_ipc_post_param(uint8_t param_id, uint32_t value);

#endif /* APP_IPC_H */
