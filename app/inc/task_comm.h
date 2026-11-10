/**
 * @file    task_comm.h
 * @brief   Task truyen thong UART 2 chieu: nhan lenh tu may tinh, cai dat thong so.
 */
#ifndef TASK_COMM_H
#define TASK_COMM_H

#include "FreeRTOS.h"
#include "task.h"

BaseType_t   task_comm_create(void);
TaskHandle_t task_comm_handle(void);

#endif /* TASK_COMM_H */
