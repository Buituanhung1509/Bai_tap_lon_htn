/**
 * @file    app_param.c
 * @brief   Kho thong so he thong (xem app_param.h).
 */
#include "app_param.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "hal_hcsr04.h"
#include <string.h>

#define PARAM_LOCK_TIMEOUT_MS  50u

typedef struct {
    const char *name;
    const char *unit;
    uint32_t    min;
    uint32_t    max;
    uint32_t    def;
} param_meta_t;

static const param_meta_t s_meta[PARAM_COUNT] = {
    /* name    unit    min             max             default        */
    { "VMAX", "cm/s", PARAM_VMAX_MIN, PARAM_VMAX_MAX, PARAM_VMAX_DEF },
    { "DIST", "mm",   PARAM_DIST_MIN, PARAM_DIST_MAX, PARAM_DIST_DEF },
    { "TOUT", "ms",   PARAM_TOUT_MIN, PARAM_TOUT_MAX, PARAM_TOUT_DEF },
    { "DET",  "cm",   PARAM_DET_MIN,  PARAM_DET_MAX,  PARAM_DET_DEF  },
    { "LOG",  "",     0u,             1u,             PARAM_LOG_DEF  },
};

static volatile uint32_t s_value[PARAM_COUNT];

static int prv_lock(void)
{
    if (xParamMutex == NULL) {
        return 1;   /* chua tao mutex (dang khoi tao, chua co task) */
    }
    return (xSemaphoreTake(xParamMutex, pdMS_TO_TICKS(PARAM_LOCK_TIMEOUT_MS)) == pdTRUE) ? 1 : 0;
}

static void prv_unlock(void)
{
    if (xParamMutex != NULL) {
        (void)xSemaphoreGive(xParamMutex);
    }
}

/* Thong so nao dung trong ISR thi day xuong driver tai day */
static void prv_apply(param_id_t id, uint32_t value)
{
    if (id == PARAM_DET) {
        hal_hcsr04_set_detect_mm((uint16_t)(value * 10u));
    }
}

void app_param_init(void)
{
    uint32_t i;
    for (i = 0u; i < (uint32_t)PARAM_COUNT; i++) {
        s_value[i] = s_meta[i].def;
        prv_apply((param_id_t)i, s_value[i]);
    }
}

param_status_t app_param_set(param_id_t id, uint32_t value)
{
    if ((uint32_t)id >= (uint32_t)PARAM_COUNT) {
        return PARAM_ERR_ID;
    }
    if ((value < s_meta[id].min) || (value > s_meta[id].max)) {
        return PARAM_ERR_RANGE;
    }
    if (!prv_lock()) {
        return PARAM_ERR_BUSY;
    }
    s_value[id] = value;
    prv_apply(id, value);
    prv_unlock();
    return PARAM_OK;
}

uint32_t app_param_get(param_id_t id)
{
    if ((uint32_t)id >= (uint32_t)PARAM_COUNT) {
        return 0u;
    }
    return s_value[id];
}

param_status_t app_param_reset(void)
{
    uint32_t i;

    if (!prv_lock()) {
        return PARAM_ERR_BUSY;
    }
    for (i = 0u; i < (uint32_t)PARAM_COUNT; i++) {
        s_value[i] = s_meta[i].def;
        prv_apply((param_id_t)i, s_value[i]);
    }
    prv_unlock();
    return PARAM_OK;
}

int app_param_find(const char *name)
{
    uint32_t i;
    for (i = 0u; i < (uint32_t)PARAM_COUNT; i++) {
        if (strcmp(name, s_meta[i].name) == 0) {
            return (int)i;
        }
    }
    return -1;
}

const char *app_param_name(param_id_t id)
{
    return ((uint32_t)id < (uint32_t)PARAM_COUNT) ? s_meta[id].name : "?";
}

const char *app_param_unit(param_id_t id)
{
    return ((uint32_t)id < (uint32_t)PARAM_COUNT) ? s_meta[id].unit : "";
}

uint32_t app_param_min(param_id_t id)
{
    return ((uint32_t)id < (uint32_t)PARAM_COUNT) ? s_meta[id].min : 0u;
}

uint32_t app_param_max(param_id_t id)
{
    return ((uint32_t)id < (uint32_t)PARAM_COUNT) ? s_meta[id].max : 0u;
}
