/**
 * @file    task_comm.c
 * @brief   UART 2 chieu: nhan lenh dang text tu may tinh (ket thuc bang \r hoac \n),
 *          khong phan biet hoa/thuong.
 *
 *  HELP                 Danh sach lenh
 *  GET [ten]            Doc tat ca / 1 thong so
 *  SET <ten> <gia tri>  Ghi thong so: VMAX, DIST, TOUT, DET, LOG
 *  RESET                Tra tat ca thong so ve mac dinh
 *  STAT                 Heap trong + stack con lai cua tung task
 *  VER                  Phien ban firmware
 */
#include "task_comm.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "app_param.h"
#include "hal_uart.h"
#include "task_sensor.h"
#include "task_calc.h"
#include "task_display.h"
#include "task_button.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define CMD_LINE_MAX    40u
#define CMD_MAX_TOKENS  4u

static TaskHandle_t xCommTaskHandle = NULL;

/* Callback trong ngat USART1: day tung byte vao queue */
static void prvUartRxIsrCb(uint8_t byte, BaseType_t *pxWoken)
{
    if (xUartRxQueue != NULL) {
        (void)xQueueSendFromISR(xUartRxQueue, &byte, pxWoken);   /* day thi bo byte */
    }
}

/* Tach chuoi theo dau cach / tab (sua truc tiep tren chuoi) */
static uint8_t prvSplit(char *s, char *tok[], uint8_t max_tok)
{
    uint8_t n = 0u;

    while (*s != '\0') {
        while ((*s == ' ') || (*s == '\t')) {
            s++;
        }
        if (*s == '\0') {
            break;
        }
        if (n >= max_tok) {
            return (uint8_t)(max_tok + 1u);     /* qua nhieu tu */
        }
        tok[n++] = s;
        while ((*s != '\0') && (*s != ' ') && (*s != '\t')) {
            s++;
        }
        if (*s != '\0') {
            *s = '\0';
            s++;
        }
    }
    return n;
}

/* Chuyen chuoi so thap phan khong dau -> uint32, tra ve 0 neu sai dinh dang */
static int prvParseU32(const char *s, uint32_t *out)
{
    char *end = NULL;
    unsigned long v;

    if ((s == NULL) || !isdigit((unsigned char)*s)) {
        return 0;
    }
    v = strtoul(s, &end, 10);
    if ((end == NULL) || (*end != '\0')) {
        return 0;
    }
    /* so qua lon -> strtoul tra ULONG_MAX, se bi app_param_set bao ERR RANGE */
    *out = (uint32_t)v;
    return 1;
}

static void prvPrintParam(param_id_t id)
{
    (void)hal_uart_printf("%s=%lu %s\r\n", app_param_name(id),
                          (unsigned long)app_param_get(id), app_param_unit(id));
}

static void prvCmdHelp(void)
{
    (void)hal_uart_printf("--- LENH UART (v%s) ---\r\n", APP_FW_VERSION);
    (void)hal_uart_printf("GET            : doc tat ca thong so\r\n");
    (void)hal_uart_printf("GET <ten>      : doc 1 thong so\r\n");
    (void)hal_uart_printf("SET VMAX <n>   : nguong vi pham %lu..%lu cm/s\r\n",
                          (unsigned long)PARAM_VMAX_MIN, (unsigned long)PARAM_VMAX_MAX);
    (void)hal_uart_printf("SET DIST <n>   : khoang cach 2 moc %lu..%lu mm\r\n",
                          (unsigned long)PARAM_DIST_MIN, (unsigned long)PARAM_DIST_MAX);
    (void)hal_uart_printf("SET TOUT <n>   : timeout %lu..%lu ms\r\n",
                          (unsigned long)PARAM_TOUT_MIN, (unsigned long)PARAM_TOUT_MAX);
    (void)hal_uart_printf("SET DET <n>    : nguong phat hien %lu..%lu cm\r\n",
                          (unsigned long)PARAM_DET_MIN, (unsigned long)PARAM_DET_MAX);
    (void)hal_uart_printf("SET LOG <0|1>  : tat/bat log [DBG]\r\n");
    (void)hal_uart_printf("RESET          : ve mac dinh\r\n");
    (void)hal_uart_printf("STAT           : heap + stack cac task\r\n");
    (void)hal_uart_printf("VER            : phien ban firmware\r\n");
}

static void prvCmdGet(uint8_t ntok, char *tok[])
{
    uint32_t i;

    if (ntok == 1u) {
        for (i = 0u; i < (uint32_t)PARAM_COUNT; i++) {
            prvPrintParam((param_id_t)i);
        }
    } else if (ntok == 2u) {
        int id = app_param_find(tok[1]);
        if (id < 0) {
            (void)hal_uart_printf("ERR NAME %s\r\n", tok[1]);
        } else {
            prvPrintParam((param_id_t)id);
        }
    } else {
        (void)hal_uart_printf("ERR ARG (GET [ten])\r\n");
    }
}

static void prvCmdSet(uint8_t ntok, char *tok[])
{
    int            id;
    uint32_t       value;
    param_status_t st;

    if (ntok != 3u) {
        (void)hal_uart_printf("ERR ARG (SET <ten> <gia tri>)\r\n");
        return;
    }
    id = app_param_find(tok[1]);
    if (id < 0) {
        (void)hal_uart_printf("ERR NAME %s\r\n", tok[1]);
        return;
    }
    if (!prvParseU32(tok[2], &value)) {
        (void)hal_uart_printf("ERR VALUE %s\r\n", tok[2]);
        return;
    }

    st = app_param_set((param_id_t)id, value);
    if (st == PARAM_OK) {
        (void)hal_uart_printf("OK %s=%lu\r\n", tok[1], (unsigned long)value);
        (void)app_ipc_post_param((uint8_t)id, value);
    } else if (st == PARAM_ERR_RANGE) {
        (void)hal_uart_printf("ERR RANGE %s %lu..%lu\r\n", tok[1],
                              (unsigned long)app_param_min((param_id_t)id),
                              (unsigned long)app_param_max((param_id_t)id));
    } else {
        (void)hal_uart_printf("ERR BUSY\r\n");
    }
}

static void prvCmdReset(void)
{
    if (app_param_reset() == PARAM_OK) {
        (void)hal_uart_printf("OK DEFAULT\r\n");
        (void)app_ipc_post_param(DISP_PARAM_ALL, 0u);
    } else {
        (void)hal_uart_printf("ERR BUSY\r\n");
    }
}

static void prvPrintStack(const char *name, TaskHandle_t h)
{
    if (h != NULL) {
        (void)hal_uart_printf("  %-8s con %lu word\r\n", name,
                              (unsigned long)uxTaskGetStackHighWaterMark(h));
    }
}

static void prvCmdStat(void)
{
    (void)hal_uart_printf("Heap trong: %lu byte (thap nhat %lu)\r\n",
                          (unsigned long)xPortGetFreeHeapSize(),
                          (unsigned long)xPortGetMinimumEverFreeHeapSize());
    (void)hal_uart_printf("Stack con lai (high water mark):\r\n");
    prvPrintStack("SENSOR",  task_sensor_handle());
    prvPrintStack("CALC",    task_calc_handle());
    prvPrintStack("DISPLAY", task_display_handle());
    prvPrintStack("COMM",    task_comm_handle());
    prvPrintStack("BUTTON",  task_button_handle());
}

static void prvHandleLine(char *line)
{
    char   *tok[CMD_MAX_TOKENS];
    uint8_t ntok = prvSplit(line, tok, CMD_MAX_TOKENS);

    if (ntok == 0u) {
        return;
    }
    if (ntok > CMD_MAX_TOKENS) {
        (void)hal_uart_printf("ERR ARG\r\n");
        return;
    }

    if (strcmp(tok[0], "HELP") == 0) {
        prvCmdHelp();
    } else if (strcmp(tok[0], "GET") == 0) {
        prvCmdGet(ntok, tok);
    } else if (strcmp(tok[0], "SET") == 0) {
        prvCmdSet(ntok, tok);
    } else if ((strcmp(tok[0], "RESET") == 0) && (ntok == 1u)) {
        prvCmdReset();
    } else if ((strcmp(tok[0], "STAT") == 0) && (ntok == 1u)) {
        prvCmdStat();
    } else if ((strcmp(tok[0], "VER") == 0) && (ntok == 1u)) {
        (void)hal_uart_printf("FW v%s\r\n", APP_FW_VERSION);
    } else {
        (void)hal_uart_printf("ERR CMD (go HELP)\r\n");
    }
}

static void prvCommTask(void *pvParameters)
{
    char    line[CMD_LINE_MAX];
    uint8_t len = 0u;
    uint8_t overflow = 0u;
    uint8_t c;

    (void)pvParameters;

    hal_uart_start_rx(prvUartRxIsrCb);
    (void)hal_uart_printf("[CMD] San sang nhan lenh. Go HELP.\r\n");

    for (;;) {
        if (xQueueReceive(xUartRxQueue, &c, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        if ((c == '\r') || (c == '\n')) {
            if (overflow) {
                (void)hal_uart_printf("ERR LEN (toi da %u ky tu)\r\n", (unsigned)(CMD_LINE_MAX - 1u));
            } else if (len > 0u) {
                line[len] = '\0';
                prvHandleLine(line);
            } else {
                /* dong rong (vd \r\n) -> bo qua */
            }
            len = 0u;
            overflow = 0u;
        } else if ((c == 0x08u) || (c == 0x7Fu)) {   /* Backspace */
            if (len > 0u) {
                len--;
            }
        } else if (len < (CMD_LINE_MAX - 1u)) {
            line[len++] = (char)toupper((int)c);
        } else {
            overflow = 1u;
        }
    }
}

BaseType_t task_comm_create(void)
{
    return xTaskCreate(prvCommTask, "COMM", TASK_COMM_STACK, NULL,
                       TASK_COMM_PRIO, &xCommTaskHandle);
}

TaskHandle_t task_comm_handle(void)
{
    return xCommTaskHandle;
}
