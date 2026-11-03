/*
 * test_core.h - HWVAL test framework core (sections 5+7)
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * SRS:     TFW-001..TFW-008, ENV-004, FRT-010..FRT-013, CLI-023..CLI-031.
 *          This header is the minimal core used by the section-5 RTOS
 *          self-tests; section 7 adds result persistence, JSON, report,
 *          filters and the fixed run order.
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

/* One test case (TFW-001): id, group, title, level, operator/destructive
 * flags, timeout, verified requirement ids and the run function. */
struct tfw_case {
    const char    *id;
    const char    *group;
    const char    *title;
    tfw_level_t    level;
    uint8_t        needs_operator;
    uint8_t        destructive;
    uint32_t       timeout_ms;
    const char    *verifies;   /* requirement id list, e.g. "FRT-020"    */
    tfw_result_t (*fn)(char *reason, size_t len); /* 0 = pass, 2 = skip, ... */
};

/* Section-5 registration entry point (tests/test_rtos.c). */
void hwval_test_register_rtos(void);

/* Registration: called once at boot from each test file (TFW-002). */
void hwval_test_register(const struct tfw_case *table, size_t count);
size_t hwval_test_count(void);
const struct tfw_case *hwval_test_at(size_t index);

/* Runner body executed by the Runner task (FRT-010; TFW-006 order grows to
 * the full fixed order in section 7 — for now: every non-operator,
 * non-destructive Level-S test, once per wake-up). */
void hwval_test_all(void);

/* Progress/state queries for the CLI and the Watchdog task (FRT-012/013).
 * The Watchdog stops kicking IWDG when this reports an expired Runner. */
int hwval_test_running(void);
int hwval_test_failed(void);
int hwval_test_runner_progress_expired(void);

/* Cooperative abort (FRT-011, CLI-027): tests poll this in their loops. */
void hwval_test_request_abort(void);
int  hwval_test_abort_requested(void);

/* Session results, readable by `results`/`report` (CLI-028/029). */
typedef struct {
    tfw_result_t result;
    uint32_t     duration_ms;
    char         reason[96];
} hwval_test_result_t;

const hwval_test_result_t *hwval_test_result_of(size_t index);
#define HWVAL_REASON_BUF 96

#endif /* HWVAL_TEST_CORE_H */