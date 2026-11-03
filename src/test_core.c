/*
 * test_core.c - HWVAL test framework (registration, runner, results)
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * SRS:     TFW-001..TFW-008, TFW-010..TFW-012, TFW-020..TFW-022,
 *          CLI-024..CLI-026, SAFE-005, FRT-011/013.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f7xx_hal.h"
#include "test_core.h"
#include "uart3.h"
#include "settings.h"

#define HWVAL_MAX_TESTS 128
#define PERSIST_MAGIC   0x48565202U /* "HVR" v2 */

/* run-order key (TFW-006): S, L, E, destructive, MCU-resetting last. */
static uint32_t order_key(const struct tfw_case *c)
{
    return (uint32_t)c->resets_mcu * 10U + (uint32_t)c->destructive * 5U +
           (uint32_t)c->level;
}

struct run_cfg {
    const char *sel;                 /* id, group or "all"                 */
    uint8_t     level_ok[3];         /* S/L/E enable bits (CLI-025)        */
    uint8_t     operator_ok;         /* default 1                          */
    uint8_t     destructive_ok;      /* default 0 (SAFE-005)               */
    uint8_t     boot_mode;           /* auto suite skips operator+destruct */
    uint32_t    repeat;
    int         until_fail;
    uint32_t    executed;
};

static struct {
    const struct tfw_case *src;
    size_t                 src_count;
    struct tfw_case        order[HWVAL_MAX_TESTS]; /* fixed, sorted view  */
    size_t                 count;
    volatile int           running;
    volatile int           failed;
    volatile uint32_t      last_progress_tick;
    volatile int           abort_requested;
    volatile uint32_t      current_timeout_ms;
} s;

/* TFW-022: results survive reset in .noinit, guarded by magic and CRC. */
static __attribute__((section(".noinit")))
struct {
    uint32_t magic;
    uint32_t crc;
    size_t   count;
    const struct tfw_case *owner[HWVAL_MAX_TESTS]; /* flash addresses    */
    hwval_test_result_t row[HWVAL_MAX_TESTS];      /* owner duplicated   */
} s_persist;

/* ------------------------------------------------------------ helpers -- */
static void result_fill(size_t i, tfw_result_t r, uint32_t ms,
                        const struct tfw_out *o)
{
    s_persist.owner[i] = &s.order[i];
    s_persist.row[i].owner = &s.order[i];
    s_persist.row[i].result = r;
    s_persist.row[i].duration_ms = ms;
    (void)snprintf(s_persist.row[i].reason, sizeof(s_persist.row[i].reason),
                   "%s", (o != NULL) ? o->reason : "");
    (void)snprintf(s_persist.row[i].measured,
                   sizeof(s_persist.row[i].measured), "%s",
                   (o != NULL) ? o->measured : "");
    (void)snprintf(s_persist.row[i].limits,
                   sizeof(s_persist.row[i].limits), "%s",
                   (o != NULL) ? o->limits : "");
}

static const char *res_name(tfw_result_t r)
{
    static const char *const names[] = { "PASS", "FAIL", "SKIP", "N/A" };
    return names[r];
}

/* TFW-010/011/012: one start-to-end line, atomic in the TX ring. */
static void result_emit_line(const struct tfw_case *c,
                             const hwval_test_result_t *r)
{
    char line[384];

    if (g_set.format == HWVAL_FMT_JSON) {
        (void)snprintf(line, sizeof(line),
            "{\"id\":\"%s\",\"group\":\"%s\",\"result\":\"%s\","
            "\"duration_ms\":%lu,\"reason\":\"%s\",\"measured\":\"%s\","
            "\"limits\":\"%s\",\"reqs\":\"%s\"}\r\n",
            c->id, c->group, res_name(r->result),
            (unsigned long)r->duration_ms, r->reason, r->measured,
            r->limits, c->verifies);
    } else {
        const char *col = (r->result == TFW_FAIL && g_set.color) ? "\033[31m" :
                          (r->result == TFW_SKIP && g_set.color) ? "\033[33m" :
                          (r->result == TFW_PASS && g_set.color) ? "\033[32m" : "";
        const char *off = (col[0] != '\0') ? "\033[0m" : "";
        (void)snprintf(line, sizeof(line), "RESULT %s %s %lu %s%s%s%s\r\n",
                       c->id, res_name(r->result),
                       (unsigned long)r->duration_ms, col, r->reason, off,
                       "");
    }
    hwval_uart3_write(line, 0U);
}

/* ------------------------------------------------------- printf helpers */
void hwval_fr(char *dst, size_t n, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    (void)vsnprintf(dst, n, fmt, ap);
    va_end(ap);
}

void hwval_fail(struct tfw_out *o, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    (void)vsnprintf(o->reason, sizeof(o->reason), fmt, ap);
    va_end(ap);
}

void hwval_meas(struct tfw_out *o, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    (void)vsnprintf(o->measured, sizeof(o->measured), fmt, ap);
    va_end(ap);
}

void hwval_lim(struct tfw_out *o, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    (void)vsnprintf(o->limits, sizeof(o->limits), fmt, ap);
    va_end(ap);
}

/* ---------------------------------------------------- CRC32 (TFW-022) -- */
uint32_t crc32_buf(const uint8_t *p, size_t n)
{
    uint32_t crc = 0xFFFFFFFFU;

    while (n-- > 0U) {
        crc ^= (uint32_t)*p++;
        for (int b = 0; b < 8; b++) {
            crc = (crc >> 1U) ^ (0xEDB88320U & (uint32_t)(-(int32_t)(crc & 1U)));
        }
    }
    return ~crc;
}

uint32_t hwval_results_crc(void)
{
    /* CRC covers count + owners + rows (everything but magic and crc). */
    return crc32_buf((const uint8_t *)&s_persist.count,
                     sizeof(s_persist) - 2U * sizeof(uint32_t));
}

void hwval_results_clear(void)
{
    (void)memset(&s_persist, 0, sizeof(s_persist));
    s_persist.magic = PERSIST_MAGIC;
    s_persist.crc = hwval_results_crc();
}

void hwval_results_restore_or_zero(void)
{
    if ((s_persist.magic != PERSIST_MAGIC) ||
        (s_persist.crc != hwval_results_crc()) ||
        (s_persist.count != s.count)) {
        (void)memset(&s_persist, 0, sizeof(s_persist));
        s_persist.magic = PERSIST_MAGIC;
        s_persist.crc = hwval_results_crc();
        return;
    }
    /* Owners must still point at the same (rebuilt) table entries. */
    for (size_t i = 0U; i < s.count; i++) {
        if ((s_persist.owner[i] != &s.order[i]) ||
            (s_persist.row[i].owner != &s.order[i])) {
            (void)memset(&s_persist, 0, sizeof(s_persist));
            s_persist.magic = PERSIST_MAGIC;
            s_persist.crc = hwval_results_crc();
            return;
        }
    }
    hwval_uart3_puts("results: restored previous session table (TFW-022)");
}

/* ---------------------------------------------------- registration ----- */
void hwval_test_register(const struct tfw_case *table, size_t count)
{
    s.src = table;
    s.src_count = count;
    s.count = 0U;

    for (size_t i = 0U; i < count; i++) { /* stable insert, TFW-006 */
        size_t j = s.count;
        while ((j > 0U) &&
               (order_key(&s.order[j - 1U]) > order_key(&table[i]))) {
            s.order[j] = s.order[j - 1U];
            j--;
        }
        s.order[j] = table[i];
        s.count++;
    }
    hwval_results_restore_or_zero();
}

size_t hwval_test_count(void) { return s.count; }
const struct tfw_case *hwval_test_at(size_t i)
{ return (i < s.count) ? &s.order[i] : NULL; }
const hwval_test_result_t *hwval_test_result_of(size_t i)
{ return (i < s.count) ? &s_persist.row[i] : NULL; }

/* ------------------------------------------------------- state queries - */
int hwval_test_running(void) { return s.running; }
int hwval_test_failed(void)  { return s.failed; }
int hwval_test_abort_requested(void) { return s.abort_requested; }
void hwval_test_request_abort(void)  { s.abort_requested = 1; }

int hwval_test_runner_progress_expired(void) /* FRT-013 */
{
    uint32_t now    = (uint32_t)xTaskGetTickCount();
    uint32_t margin = s.running ? (s.current_timeout_ms + 5000U) : 5000U;

    return (int)((now - s.last_progress_tick) > margin);
}

/* ----------------------------------------------------------- the runner */
static int cfg_allows(const struct run_cfg *cfg, const struct tfw_case *c)
{
    if ((cfg->sel != NULL) && (strcmp(cfg->sel, "all") != 0) &&
        (strcmp(c->id, cfg->sel) != 0) && (strcmp(c->group, cfg->sel) != 0)) {
        return 0;
    }
    if (cfg->level_ok[(int)c->level] == 0U) {
        return 0;
    }
    if (cfg->boot_mode) { /* SAFE-005 boot defaults */
        return ((c->needs_operator == 0U) && (c->destructive == 0U)) ? 1 : 0;
    }
    if ((c->needs_operator != 0U) && (cfg->operator_ok == 0U)) {
        return 0;
    }
    if ((c->destructive != 0U) && (cfg->destructive_ok == 0U)) {
        return 0;
    }
    return 1;
}

static void run_case(size_t i, const struct run_cfg *cfg)
{
    const struct tfw_case *c = &s.order[i];
    struct tfw_out out;
    TickType_t t0;
    tfw_result_t r;

    s.current_timeout_ms  = (c->timeout_ms != 0U) ? c->timeout_ms : 2000U;
    s.last_progress_tick  = (uint32_t)xTaskGetTickCount();
    (void)memset(&out, 0, sizeof(out));

    /* Skip reasons first (ENV-002/003 checks live inside every test). */
    if (cfg->boot_mode) {
        if ((c->needs_operator != 0U) || (c->destructive != 0U)) {
            struct tfw_out so = { {0}, {0}, {0} };
            (void)snprintf(so.reason, sizeof(so.reason),
                           "boot suite excludes operator/destructive");
            result_fill(i, TFW_SKIP, 0U, &so);
            result_emit_line(c, &s_persist.row[i]);
            return;
        }
    } else {
        if ((c->needs_operator != 0U) && (cfg->operator_ok == 0U)) {
            struct tfw_out so = { {0}, {0}, {0} };
            (void)snprintf(so.reason, sizeof(so.reason),
                           "NO_OPERATOR (--no-operator filter)");
            result_fill(i, TFW_SKIP, 0U, &so);
            result_emit_line(c, &s_persist.row[i]);
            return;
        }
        if ((c->destructive != 0U) && (cfg->destructive_ok == 0U)) {
            struct tfw_out so = { {0}, {0}, {0} };
            (void)snprintf(so.reason, sizeof(so.reason),
                           "NO_DESTRUCTIVE_FLAG (SAFE-005)");
            result_fill(i, TFW_SKIP, 0U, &so);
            result_emit_line(c, &s_persist.row[i]);
            return;
        }
    }
    if (hwval_test_abort_requested()) {
        struct tfw_out ao = { {0}, {0}, {0} };
        (void)snprintf(ao.reason, sizeof(ao.reason), "abort");
        result_fill(i, TFW_SKIP, 0U, &ao);
        result_emit_line(c, &s_persist.row[i]);
        return;
    }

    t0 = xTaskGetTickCount();
    s.running = 1;
    ((struct run_cfg *)(uintptr_t)cfg)->executed++;    r = c->fn(&out);
    s.running = 0;
    result_fill(i, r, (uint32_t)(xTaskGetTickCount() - t0), &out);
    if (r == TFW_FAIL) {
        s.failed++;
    }
    s.last_progress_tick = (uint32_t)xTaskGetTickCount();
    result_emit_line(c, &s_persist.row[i]);
}

static void run_suite(const struct run_cfg *cfg)
{
    s.failed = 0;
    s.abort_requested = 0;
    s.last_progress_tick = (uint32_t)xTaskGetTickCount();

    for (size_t i = 0U; i < s.count; i++) {
        if (cfg_allows(cfg, &s.order[i]) == 0) {
            continue;
        }
        run_case(i, cfg);
        if (hwval_test_abort_requested()) {
            break;
        }
        if ((cfg->until_fail != 0) && (s.failed > 0)) {
            break;
        }
    }
    s.running = 0;
}

void hwval_test_all(void) /* boot suite (SAFE-005): no operator/destructive */
{
    struct run_cfg cfg = { 0 };

    cfg.sel            = "all";
    cfg.level_ok[0]    = 1U;
    cfg.level_ok[1]    = 1U;
    cfg.level_ok[2]    = 1U;
    cfg.operator_ok    = 0U;
    cfg.destructive_ok = 0U;
    cfg.boot_mode      = 1U;
    run_suite(&cfg);
}

/* ------------------------------------------- CLI run flags (CLI-025/026) */
static void parse_level_bits(const char *tok, uint8_t *ok3)
{
    for (; *tok != '\0'; tok++) {
        if ((*tok == 'S') || (*tok == 's')) { ok3[TFW_LEVEL_S] = 1U; }
        else if ((*tok == 'L') || (*tok == 'l')) { ok3[TFW_LEVEL_L] = 1U; }
        else if ((*tok == 'E') || (*tok == 'e')) { ok3[TFW_LEVEL_E] = 1U; }
    }
}

int hwval_test_run_sel(const char *selector, const char *flags)
{
    struct run_cfg cfg = { 0 };
    uint32_t total = 0U;
    char flagbuf[128];
    char *tok;
    char *save = NULL;

    cfg.sel            = selector;
    cfg.level_ok[0]    = 1U; /* defaults: all levels        */
    cfg.level_ok[1]    = 1U;
    cfg.level_ok[2]    = 1U;
    cfg.operator_ok    = 1U; /* operator allowed            */
    cfg.destructive_ok = 0U; /* destructive NOT (SAFE-005)  */
    cfg.repeat         = 1U;

    if (flags != NULL) {
        (void)snprintf(flagbuf, sizeof(flagbuf), "%s", flags);
        tok = strtok_r(flagbuf, " \t", &save);
        while (tok != NULL) {
            if (strncmp(tok, "--level ", 8U) == 0) {
                cfg.level_ok[0] = cfg.level_ok[1] = cfg.level_ok[2] = 0U;
                parse_level_bits(tok + 8U, cfg.level_ok);
            } else if (strcmp(tok, "--no-operator") == 0) {
                cfg.operator_ok = 0U;
            } else if (strcmp(tok, "--destructive") == 0) {
                cfg.destructive_ok = 1U; /* SAFE-005: only with flag */
            } else if (strcmp(tok, "--no-destructive") == 0) {
                cfg.destructive_ok = 0U;
            } else if (strcmp(tok, "--until-fail") == 0) {
                cfg.until_fail = 1;
            } else if (strncmp(tok, "--repeat ", 9U) == 0) {
                cfg.repeat = (uint32_t)strtoul(tok + 9U, NULL, 10);
                if (cfg.repeat == 0U) { cfg.repeat = 1U; }
            }
            tok = strtok_r(NULL, " \t", &save);
        }
    }

    if ((cfg.level_ok[0] | cfg.level_ok[1] | cfg.level_ok[2]) == 0U) {
        return 0; /* nonsensical filter */
    }

    for (uint32_t rep = 0U; rep < cfg.repeat; rep++) {
        run_suite(&cfg);
        total += cfg.executed;
        if (cfg.until_fail && (s.failed > 0)) {
            break;
        }
    }
    return (int)total;
}

/* ------------------------------------------------- TFW-007: bootcheck -- */
#define BOOTCHECK_MAGIC 0xB0C4U

static void bkp_unlock(void)
{
    __HAL_RCC_PWR_CLK_ENABLE();
    SET_BIT(PWR->CR1, PWR_CR1_DBP);
    SET_BIT(RCC->APB1ENR, RCC_APB1ENR_RTCEN);
    (void)RCC->APB1ENR;
    RTC->WPR = 0xCAU;
    RTC->WPR = 0x53U;
}

static void bkp_lock(void)
{
    RTC->WPR = 0xFFU;
}

void hwval_bootcheck_arm(uint32_t case_hash, tfw_result_t expect)
{
    bkp_unlock();
    RTC->BKP0R = BOOTCHECK_MAGIC;
    RTC->BKP1R = case_hash;
    RTC->BKP2R = (uint32_t)expect;
    bkp_lock();
}

void hwval_bootcheck_evaluate(void)
{
    char line[128];

    if (!hwval_uart3_ready()) {
        return;
    }
    bkp_unlock();
    if (RTC->BKP0R != BOOTCHECK_MAGIC) {
        bkp_lock();
        return;
    }
    uint32_t hash   = RTC->BKP1R;
    tfw_result_t got = (tfw_result_t)RTC->BKP2R;

    if (hash == 0x5A00U) { /* RAM.009: backup-SRAM retention check */
        volatile const uint32_t *b = (const uint32_t *)0x40024000U;
        got = ((b[0] == RTC->BKP3R) && (b[1] == RTC->BKP4R)) ? TFW_PASS
                                                            : TFW_FAIL;
        (void)snprintf(line, sizeof(line),
                       "BOOTCHECK RAM.009 bkpsram=%08lX%08lX -> %s\r\n",
                       (unsigned long)b[1], (unsigned long)b[0],
                       res_name(got));
        RTC->BKP0R = 0U; RTC->BKP1R = 0U; RTC->BKP2R = 0U;
        bkp_lock();
        hwval_uart3_write(line, 0U);
        return;
    }
    RTC->BKP0R = 0U;
    RTC->BKP1R = 0U;
    RTC->BKP2R = 0U;
    bkp_lock();

    (void)snprintf(line, sizeof(line),
                   "BOOTCHECK hash=%04lX result=%s (TFW-007 post-reset)\r\n",
                   (unsigned long)(hash & 0xFFFFU), res_name(got));
    hwval_uart3_write(line, 0U);
}