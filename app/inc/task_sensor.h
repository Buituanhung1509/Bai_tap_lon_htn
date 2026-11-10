/**
 * @file    task_sensor.h
 * @brief   Task phat xung Trig cho 2 cam bien HC-SR04 theo chu ky SENSOR_PERIOD_MS.
 */
#ifndef TASK_SENSOR_H
#define TASK_SENSOR_H

#include "FreeRTOS.h"
#include "task.h"

BaseType_t   task_sensor_create(void);
TaskHandle_t task_sensor_handle(void);

#endif /* TASK_SENSOR_H */
