/**
 * @file    task_calc.h
 * @brief   Task tinh toc do: nhan moc tu xSensorQueue, tinh v = S/dt, kiem tra timeout.
 */
#ifndef TASK_CALC_H
#define TASK_CALC_H

#include "FreeRTOS.h"
#include "task.h"

BaseType_t   task_calc_create(void);
TaskHandle_t task_calc_handle(void);

#endif /* TASK_CALC_H */
