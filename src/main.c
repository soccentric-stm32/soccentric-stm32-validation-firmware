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
    hwval_iwdg_start(HWVAL_IWDG_TIMEOUT_MS); /* GEN-009                      */

    hwval_uart3_puts("HWVAL ready (section 4: boot path; CLI arrives in"
                     " section 6)\r\n");

    /* LD1 blink while the scheduler does not exist yet (FRT-012 takes over
     * this LED in section 5). */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef led = {
        .Pin = HWVAL_LED1_PIN, .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL, .Speed = GPIO_SPEED_FREQ_LOW
    };
    HAL_GPIO_Init(HWVAL_LED1_PORT, &led);

    for (;;) {
        HAL_GPIO_TogglePin(HWVAL_LED1_PORT, HWVAL_LED1_PIN);
        HAL_Delay(250U);
        hwval_iwdg_kick();
    }
}