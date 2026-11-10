/**
 * @file    app_cfg.h
 * @brief   Bang tai nguyen RTOS va thong so ung dung (thong nhat truoc khi code task).
 */
#ifndef APP_CFG_H
#define APP_CFG_H

#include "FreeRTOS.h"

#define APP_FW_VERSION          "2.0.0"

/* ======================= Task priority =======================
 * SENSOR > COMM = BUTTON > CALC > DISPLAY
 * - SENSOR cao nhat: nhip phat Trig deu dan (moc thoi gian lay trong ISR nen
 *   do chinh xac khong phu thuoc uu tien task).
 * - COMM/BUTTON: phan hoi nguoi dung nhanh, phan lon thoi gian bi block.
 * - DISPLAY thap nhat: LCD I2C cham, khong duoc chan cac task khac.          */
#define TASK_SENSOR_PRIO        (tskIDLE_PRIORITY + 4)
#define TASK_COMM_PRIO          (tskIDLE_PRIORITY + 3)
#define TASK_BUTTON_PRIO        (tskIDLE_PRIORITY + 3)
#define TASK_CALC_PRIO          (tskIDLE_PRIORITY + 2)
#define TASK_DISPLAY_PRIO       (tskIDLE_PRIORITY + 1)

/* ======================= Stack size (word = 4 byte) =======================
 * Task co goi hal_uart_printf (vsnprintf) can >= 192 word.
 * Kiem tra muc dung that bang lenh UART "STAT".                             */
#define TASK_SENSOR_STACK       128u
#define TASK_COMM_STACK         320u
#define TASK_BUTTON_STACK       192u
#define TASK_CALC_STACK         256u
#define TASK_DISPLAY_STACK      256u

/* ======================= Queue length ======================= */
#define Q_SENSOR_LEN            8u     /* hcsr04_event_t   */
#define Q_DISPLAY_LEN           6u     /* disp_msg_t       */
#define Q_UART_RX_LEN           64u    /* uint8_t (1 byte) */

/* ======================= Thong so mac dinh + gioi han =======================
 * Cac gia tri nay co the doi luc chay qua UART (SET ...) hoac nut nhan.     */
#define PARAM_VMAX_DEF          40u    /* nguong toc do vi pham (cm/s) */
#define PARAM_VMAX_MIN          10u
#define PARAM_VMAX_MAX          200u

#define PARAM_DIST_DEF          130u   /* khoang cach 2 moc (mm) */
#define PARAM_DIST_MIN          50u
#define PARAM_DIST_MAX          500u

#define PARAM_TOUT_DEF          3000u  /* thoi gian cho moc thu 2 (ms) */
#define PARAM_TOUT_MIN          500u
#define PARAM_TOUT_MAX          10000u

#define PARAM_DET_DEF           15u    /* nguong phat hien vat (cm) */
#define PARAM_DET_MIN           3u
#define PARAM_DET_MAX           30u

#define PARAM_LOG_DEF           1u     /* 1 = in them dong [DBG] */

/* ======================= Nut nhan ======================= */
#define BTN_DEBOUNCE_MS         30u
#define BTN_POLL_MS             10u
#define BTN_LONG_MS             1500u  /* giu >= 1.5 s -> ve mac dinh */
#define BTN_VMAX_STEP           10u    /* nhan ngan: VMAX += 10 */
#define BTN_VMAX_LOW            20u    /* vuot BTN_VMAX_HIGH thi quay ve gia tri nay */
#define BTN_VMAX_HIGH           100u

/* ======================= Do / hien thi ======================= */
#define SENSOR_PERIOD_MS        40u    /* chu ky phat Trig (echo toi da ~38 ms) */
#define CALC_POLL_MS            50u    /* chu ky kiem tra timeout */
#define CALC_MIN_DT_US          50000u /* dt < 50 ms coi la nhieu, bo qua */
#define DISP_RESULT_HOLD_MS     2000u  /* giu ket qua tren LCD */
#define DISP_PARAM_HOLD_MS      1500u  /* giu man hinh "CAI DAT" */
#define DISP_BEEP_MS            60u    /* tieng bip xac nhan khi doi thong so */

/* 1: luc khoi dong quet bus I2C va in dia chi tim thay qua UART */
#define I2C_SCAN_ENABLE         1

#endif /* APP_CFG_H */
