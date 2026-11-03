/*
 * test_core.c - HWVAL test framework core
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * SRS:     TFW-001..TFW-008 (minimal core; section 7 adds persistence,
 *          text/JSON output, report, filters, fixed order).
 */

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "test_core.h"
#include "uart3.h"

#define HWVAL_MAX_TESTS 128

static struct {
    const struct tfw_case *table;
    size_t                 count;
    volatile int           running;
    volatile int           failed;
    volatile uint32_t      last_progress_tick;
    volatile int           abort_requested;
    volatile uint32_t      current_timeout_ms;
    hwval_test_result_t    results[HWVAL_MAX_TESTS];
} s;

void hwval_test_register(const struct tfw_case *table, size_t count)
{
    s.table = table;
    s.count = count;
    for (size_t i = 0U; (i < count) && (i < HWVAL_MAX_TESTS); i++) {
        s.results[i].result = TFW_PASS;
        s.results[i].duration_ms = 0U;
        s.results[i].reason[0] = '\0';
    }
}

size_t hwval_test_count(void) { return s.count; }
const struct tfw_case *hwval_test_at(size_t i)
{ return (i < s.count) ? &s.table[i] : NULL; }
const hwval_test_result_t *hwval_test_result_of(size_t i)
{ return (i < s.count) ? &s.results[i] : NULL; }

int hwval_test_running(void) { return s.running; }
int hwval_test_failed(void)  { return s.failed; }
int hwval_test_abort_requested(void) { return s.abort_requested; }
void hwval_test_request_abort(void)  { s.abort_requested = 1; }

/* FRT-013: expired = the Runner has not advanced for more than the current
 * test's timeout + 5 s. */
int hwval_test_runner_progress_expired(void)
{
    uint32_t now    = (uint32_t)xTaskGetTickCount();
    uint32_t margin = s.running ? (s.current_timeout_ms + 5000U) : 5000U;

    return (int)((now - s.last_progress_tick) > margin);
}

static void print_result(const struct tfw_case *c, tfw_result_t r,
                         uint32_t ms, const char *reason)
{
    static const char *const names[] = { "PASS", "FAIL", "SKIP", "N/A" };
    char line[176];

    (void)snprintf(line, sizeof(line), "RESULT %s %s %lu %s\r\n",
                   c->id, names[r], (unsigned long)ms,
                   (reason != NULL) ? reason : "");
    hwval_uart3_write(line, 0U);
}

/* Run every non-operator, non-destructive Level-S test (order rules grow
 * in section 7 per TFW-006). */
void hwval_test_all(void)
{
    static char reason[HWVAL_REASON_BUF];

    s.failed = 0;
    s.running = 1;
    s.abort_requested = 0;
    s.last_progress_tick = (uint32_t)xTaskGetTickCount();

    for (size_t i = 0U; (i < s.count) && (i < HWVAL_MAX_TESTS); i++) {
        const struct tfw_case *c = &s.table[i];
        TickType_t t0 = xTaskGetTickCount();

        s.current_timeout_ms  = (c->timeout_ms != 0U) ? c->timeout_ms : 2000U;
        s.last_progress_tick  = (uint32_t)t0;
        reason[0] = '\0'; /* no stale reasons (tests write only on failure) */

        if ((c->needs_operator != 0U) || (c->destructive != 0U)) {
            s.results[i].result = TFW_SKIP;
            (void)snprintf(reason, sizeof(reason),
                           "skipped (operator/destructive)");
            s.results[i].duration_ms = 0U;
        } else {
            tfw_result_t r = c->fn(reason, HWVAL_REASON_BUF);

            s.results[i].result = r;
            s.results[i].duration_ms =
                (uint32_t)(xTaskGetTickCount() - t0);
            if (r == TFW_FAIL) {
                s.failed = 1;
            }
        }
        (void)snprintf(s.results[i].reason,
                       sizeof(s.results[i].reason), "%s", reason);
        s.last_progress_tick = (uint32_t)xTaskGetTickCount();

        print_result(c, s.results[i].result, s.results[i].duration_ms,
                     reason);

        if (s.abort_requested) {
            (void)snprintf(reason, sizeof(reason), "abort");
            break;
        }
    }

    s.running = 0;
}