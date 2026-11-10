/**
 * @file    task_display.h
 * @brief   Task hien thi: so huu LCD + den/coi, ghi log ket qua qua UART.
 */
#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

#include "FreeRTOS.h"
#include "task.h"

BaseType_t   task_display_create(void);
TaskHandle_t task_display_handle(void);

#endif /* TASK_DISPLAY_H */
