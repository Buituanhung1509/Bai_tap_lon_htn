/**
 * @file    task_button.h
 * @brief   Task nut nhan: chong rung, nhan ngan tang VMAX, giu lau ve mac dinh.
 */
#ifndef TASK_BUTTON_H
#define TASK_BUTTON_H

#include "FreeRTOS.h"
#include "task.h"

BaseType_t   task_button_create(void);
TaskHandle_t task_button_handle(void);

#endif /* TASK_BUTTON_H */
