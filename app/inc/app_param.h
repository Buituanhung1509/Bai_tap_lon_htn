/**
 * @file    app_param.h
 * @brief   Kho thong so he thong - noi DUY NHAT luu cac thong so cai dat duoc.
 *          Ghi (set/reset): co mutex + kiem tra gioi han.
 *          Doc (get): doc bien 32-bit can le la nguyen tu tren Cortex-M3,
 *          nen khong can lay mutex.
 */
#ifndef APP_PARAM_H
#define APP_PARAM_H

#include <stdint.h>

typedef enum {
    PARAM_VMAX = 0,   /* cm/s */
    PARAM_DIST,       /* mm   */
    PARAM_TOUT,       /* ms   */
    PARAM_DET,        /* cm   */
    PARAM_LOG,        /* 0/1  */
    PARAM_COUNT
} param_id_t;

typedef enum {
    PARAM_OK = 0,
    PARAM_ERR_RANGE,
    PARAM_ERR_ID,
    PARAM_ERR_BUSY
} param_status_t;

void           app_param_init(void);
param_status_t app_param_set(param_id_t id, uint32_t value);
uint32_t       app_param_get(param_id_t id);
param_status_t app_param_reset(void);

int            app_param_find(const char *name);   /* tra ve id hoac -1 */
const char    *app_param_name(param_id_t id);
const char    *app_param_unit(param_id_t id);
uint32_t       app_param_min(param_id_t id);
uint32_t       app_param_max(param_id_t id);

#endif /* APP_PARAM_H */
