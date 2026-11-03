/*
 * hwval - firmware entry point
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * Target:  NUCLEO-F767ZI (STM32F767ZIT6)
 * SRS:     GEN-002, GEN-003, GEN-004, GEN-008..GEN-010, GEN-012
 *
 * Section 4 of the SRS: boot sequence and general firmware behaviour.
 * The FreeRTOS tasks (section 5), the CLI (section 6) and the test
 * framework (section 7) attach in later SRS sections.
 */

#include <stdio.h>
#include <string.h>
#include "stm32f7xx_hal.h"
#include "hwval_board.h"
#include "clock.h"
#include "version.h"
#include "uart3.h"
#include "fault.h"
#include "iwdg.h"
#include "reset_cause.h"
#include "mpu_setup.h"
#include "rtos.h"
#include "test_core.h"

static void report_previous_fault(void)
{
    const hwval_fault_record_t *r = hwval_fault_record();
    char line[96];

    if (r->magic != 0x4856464CU /* "HVFL" */ || r->count == 0U) {
        return;
    }
    (void)snprintf(line, sizeof(line),
        "boot: previous fault #%lu CFSR=%08lX HFSR=%08lX PC=%08lX\r\n",
        (unsigned long)r->count, (unsigned long)r->cfsr,
        (unsigned long)r->hfsr, (unsigned long)r->pc);
    hwval_uart3_write(line, strlen(line));
}

int main(void)
{
    hwval_mpu_setup();          /* GEN-007: non-cacheable DMA region, OTP RO */
    HAL_Init();                 /* HAL time base on TIM6 (GEN-012)           */
    hwval_clock_init();         /* GEN-004: 216 MHz, scale 1, OD, 7 WS       */
    hwval_uart3_init();         /* VCP console (USART3, section 6 builds on) */
    hwval_version_banner();     /* GEN-003                                   */
    hwval_reset_cause_report(); /* GEN-010                                   */
    report_previous_fault();    /* GEN-008 evidence after fault reset        */
    hwval_bootcheck_evaluate(); /* TFW-007: post-reset test verdicts         */
    hwval_rtos_report_panic();  /* FRT-004/005 hook evidence after reset     */
    hwval_iwdg_start(HWVAL_IWDG_TIMEOUT_MS); /* GEN-009                      */

    hwval_uart3_puts("HWVAL: boot ok, starting RTOS tasks (section 5)\r\n");

    hwval_test_register_rtos(); /* RTOS self-tests (section 5.3)             */
    hwval_test_register_dev();  /* device identity (section 8.1)             */
    hwval_test_register_cm7();  /* Cortex-M7 core (section 8.2)              */
    hwval_test_register_cache_mpu(); /* caches/TCM/MPU (section 8.3)          */
    hwval_test_register_ram();  /* internal RAM (section 8.4)                */
    hwval_test_register_flash();/* flash/OB/OTP (section 8.5)                */
    hwval_test_register_clk();  /* clocks (section 8.6)                      */
    hwval_test_register_pwr();  /* power (section 8.7)                       */
    hwval_test_register_rst_wdg(); /* resets + watchdogs (sections 8.8/8.9)  */
    hwval_rtos_start();         /* FRT-010: cli/runner/hb/wdg + scheduler    */

    /* Unreachable: the scheduler never returns. */
    hwval_fault_assert(__FILE__, __LINE__);
    return 0;
}