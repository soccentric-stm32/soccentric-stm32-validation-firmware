/*
 * test_core.h - HWVAL test framework (sections 5+7)
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * SRS:     TFW-001..TFW-008, TFW-010..TFW-022, ENV-004, FRT-010..FRT-013,
 *          CLI-023..CLI-035, TFW-007 (bootcheck protocol).
 */

#ifndef HWVAL_TEST_CORE_H
#define HWVAL_TEST_CORE_H

#include <stddef.h>
#include <stdint.h>

/* Result states (TFW-001). */
typedef enum {
    TFW_PASS = 0,
    TFW_FAIL = 1,
    TFW_SKIP = 2,
    TFW_NA   = 3
} tfw_result_t;

/* Test levels (section 3.2). */
typedef enum { TFW_LEVEL_S = 0, TFW_LEVEL_L = 1, TFW_LEVEL_E = 2 } tfw_level_t;

/* What a test writes: details for the FAIL/SKIP line (TFW-004) and the
 * machine-readable measured/limits fields (TFW-011). */
struct tfw_out {
    char reason[96];
    char measured[48];
    char limits[48];
};

#define HWVAL_REASON_BUF 96

/* printf-style helpers every test uses. */
void hwval_fr(char *dst, size_t n, const char *fmt, ...);           /* reason   */
void hwval_fail(struct tfw_out *o, const char *fmt, ...);           /* FAIL why */
void hwval_meas(struct tfw_out *o, const char *fmt, ...);           /* measured */
void hwval_lim(struct tfw_out *o, const char *fmt, ...);            /* limits   */

/* One test case (TFW-001). */
struct tfw_case {
    const char    *id;
    const char    *group;
    const char    *title;
    tfw_level_t    level;
    uint8_t        needs_operator;
    uint8_t        destructive;
    uint8_t        resets_mcu;   /* TFW-006/007: runs last, bootcheck arm  */
    uint32_t       timeout_ms;
    const char    *verifies;     /* requirement ids, e.g. "FRT-020"        */
    tfw_result_t (*fn)(struct tfw_out *o);
};

/* Registration: each test file calls hwval_test_register() once at boot
 * (TFW-002 single table). The framework sorts into the fixed order of
 * TFW-006: S, L, E, destructive, MCU-resetting last. */
void hwval_test_register(const struct tfw_case *table, size_t count);

/* Registration entry points, one per test file (called from main). */
void hwval_test_register_rtos(void);
void hwval_test_register_dev(void);
void hwval_test_register_cm7(void);
void hwval_test_register_cache_mpu(void);
void hwval_test_register_ram(void);
void hwval_test_register_flash(void);
void hwval_test_register_clk(void);
void hwval_test_register_pwr(void);
void hwval_test_register_rst_wdg(void);
void hwval_test_register_rtc(void);
void hwval_test_register_int(void);
void hwval_test_register_dma(void);
void hwval_test_register_gpio(void);
void hwval_test_register_led_btn(void);
void hwval_test_register_adc(void);
void hwval_test_register_dac(void);
void hwval_test_register_tim(void);
void hwval_test_register_uart(void);
void hwval_test_register_spi(void);

size_t hwval_test_count(void);
const struct tfw_case *hwval_test_at(size_t index);

/* Runner body executed by the Runner task: the fixed-order suite minus
 * operator/destructive cases (SAFE-005) evaluated preconditions of every
 * case itself (ENV-002/ENV-003). */
void hwval_test_all(void);

/* `run <id|group|all> [flags]` (CLI-024..CLI-026): flags string holds
 * space-separated options: --level S,L,E --no-operator --destructive
 * --repeat N --until-fail. Defaults: all levels, operator allowed,
 * destructive NOT allowed. Returns cases executed. */
int hwval_test_run_sel(const char *selector, const char *flags);

/* Progress/state queries for the CLI and Watchdog (FRT-012/013). */
int hwval_test_running(void);
int hwval_test_failed(void);
int hwval_test_runner_progress_expired(void);

/* Cooperative abort (FRT-011, CLI-027). Tests poll between steps; on
 * timeout the watchdog reset is the hard backstop (TFW-003). */
void hwval_test_request_abort(void);
int  hwval_test_abort_requested(void);

/* A test result row of the session (TFW-010..TFW-012, TFW-022). */
typedef struct {
    const struct tfw_case *owner;   /* id/group from the registered table */
    tfw_result_t result;
    uint32_t     duration_ms;
    char         reason[96];
    char         measured[48];
    char         limits[48];
} hwval_test_result_t;

const hwval_test_result_t *hwval_test_result_of(size_t index);

/* Reset-surviving results (TFW-022): magic + CRC in .noinit; restored at
 * boot unless `results clear` was the last command. */
void hwval_results_clear(void);
uint32_t hwval_results_crc(void);

uint32_t crc32_buf(const uint8_t *p, size_t n);

/* TFW-007: tests that reset the MCU arm this before NVIC_SystemReset and
 * the boot path reports the outcome. */
void hwval_bootcheck_arm(uint32_t case_id_hash, tfw_result_t expect);
/* Boot-time evaluation; prints the verdict banner once and clears. */
void hwval_bootcheck_evaluate(void);

#endif /* HWVAL_TEST_CORE_H */