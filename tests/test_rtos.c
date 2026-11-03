/*
 * tests/test_rtos.c - FreeRTOS self-tests (section 5.3)
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * SRS:     FRT-020..FRT-026 (registered as the RTOS test group); FRT-009
 *          lazy-FPU stacking is verified numerically by RTOS.021.FPU.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"
#include "timers.h"
#include "stream_buffer.h"
#include "message_buffer.h"
#include "stm32f7xx_hal.h"
#include "test_core.h"

/* reason helper: first failure wins */
static void fr(char *buf, size_t len, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    (void)vsnprintf(buf, len, fmt, ap);
    va_end(ap);
}

/* --------------------------------------------------------------- FRT-020 */
/* The test task increments only when notified — deterministic lifecycle
 * checks without timing races. */
static TaskHandle_t s_t20;
static volatile uint32_t s_t20_ctr;

static void t20_body(void *arg)
{
    (void)arg;
    uint32_t notified = 0U;

    for (;;) {
        if (xTaskNotifyWait(0, 0xFFFFFFFF, &notified, portMAX_DELAY) ==
            pdTRUE) {
            if (notified != 0U) {
                s_t20_ctr++;
            } else {
                break; /* 0 = exit request */
            }
        }
    }
    s_t20 = NULL;
    vTaskDelete(NULL);
}

static int notify_t20(uint32_t what)
{
    return (s_t20 != NULL) ? (xTaskNotify(s_t20, what, eSetBits) == pdPASS)
                           : 0;
}

static tfw_result_t test_tasks(char *b, size_t l)
{
    s_t20 = NULL;
    s_t20_ctr = 0U;

    BaseType_t ok = xTaskCreate(t20_body, "t20", 256, NULL, 5, &s_t20);
    if (ok != pdPASS) {
        fr(b, l, "xTaskCreate failed");
        return TFW_FAIL;
    }
    vTaskDelay(2U);

    if (!notify_t20(1)) { s_t20_ctr = 0U; }
    vTaskDelay(pdMS_TO_TICKS(30));
    uint32_t c1 = s_t20_ctr;
    if (c1 < 1U) {
        fr(b, l, "task did not run");
        vTaskDelete(s_t20);
        return TFW_FAIL;
    }

    vTaskSuspend(s_t20);
    vTaskDelay(pdMS_TO_TICKS(30));
    (void)notify_t20(1); /* ignored while suspended */
    vTaskDelay(pdMS_TO_TICKS(30));
    if (s_t20_ctr != c1) {
        fr(b, l, "suspended task still ran");
        vTaskResume(s_t20);
        vTaskDelay(pdMS_TO_TICKS(10));
        vTaskDelete(s_t20);
        return TFW_FAIL;
    }

    vTaskResume(s_t20);
    vTaskDelay(pdMS_TO_TICKS(30));
    (void)notify_t20(1);
    vTaskDelay(pdMS_TO_TICKS(30));
    if (s_t20_ctr == c1) {
        fr(b, l, "resumed task did not run");
        vTaskDelete(s_t20);
        return TFW_FAIL;
    }

    UBaseType_t saved = uxTaskPriorityGet(s_t20);
    vTaskPrioritySet(s_t20, 8);
    if (uxTaskPriorityGet(s_t20) != 8U) {
        fr(b, l, "priority set did not apply");
        vTaskPrioritySet(s_t20, saved);
        vTaskDelete(s_t20);
        return TFW_FAIL;
    }
    vTaskPrioritySet(s_t20, saved);

    TaskHandle_t h = s_t20;
    (void)notify_t20(0); /* politely exit */
    vTaskDelay(pdMS_TO_TICKS(30));
    if (s_t20 == NULL) {
        /* self-deleted already; deleting again would double-free */
        return TFW_PASS;
    }
    vTaskDelete(h);
    vTaskDelay(pdMS_TO_TICKS(30));
    if (s_t20_ctr != c1 + 1U) { /* no more increments */
        (void)notify_t20(1);
        vTaskDelay(pdMS_TO_TICKS(30));
        if (s_t20_ctr != c1 + 1U) {
            fr(b, l, "deleted task still ran");
            return TFW_FAIL;
        }
    }
    return TFW_PASS;
}

/* --------------------------------------------------------------- FRT-009 */
static volatile uint32_t s_fpa_bits, s_fpb_bits;
static volatile TaskHandle_t s_fpa_done, s_fpb_done;

/* float bit patterns make any corruption visible */
static void fpu_a(void *arg)
{
    (void)arg;
    float x = 1.0f;

    for (int i = 0; i < 200; i++) {
        x = x * 1.001f + 0.0005f;
        vTaskDelay(1U); /* forced context switches between FP ops */
    }
    { uint32_t bits; memcpy(&bits, &x, 4U); s_fpa_bits = bits; }
    s_fpa_done = xTaskGetCurrentTaskHandle();
    vTaskDelete(NULL);
}

static void fpu_b(void *arg)
{
    (void)arg;
    float x = -7.0f;

    for (int i = 0; i < 150; i++) {
        x = x / 1.002f - 0.25f;
        vTaskDelay(1U);
    }
    { uint32_t bits; memcpy(&bits, &x, 4U); s_fpb_bits = bits; }
    s_fpb_done = xTaskGetCurrentTaskHandle();
    vTaskDelete(NULL);
}

static tfw_result_t test_fpu_lazy_stacking(char *b, size_t l)
{
    s_fpa_bits = 0U; s_fpb_bits = 0U; s_fpa_done = NULL; s_fpb_done = NULL;

    if (xTaskCreate(fpu_a, "fpa", 512, NULL, 4, NULL) != pdPASS ||
        xTaskCreate(fpu_b, "fpb", 512, NULL, 4, NULL) != pdPASS) {
        fr(b, l, "task create failed");
        return TFW_FAIL;
    }
    /* FP ops in THIS task, interleaved with the workers' switches */
    float here = 3.5f;
    for (int i = 0; i < 40; i++) {
        here = here * 1.01f;
        vTaskDelay(1U);
    }

    /* wait for both workers to finish (max ~600ms of their loops) */
    for (int i = 0; (i < 700) && (s_fpa_done == NULL || s_fpb_done == NULL);
         i++) {
        vTaskDelay(1U);
    }
    if (s_fpa_done == NULL || s_fpb_done == NULL) {
        fr(b, l, "workers did not finish (a=%p b=%p)",
           (void *)s_fpa_done, (void *)s_fpb_done);
        return TFW_FAIL;
    }

    float ea = 1.0f;
    for (int i = 0; i < 200; i++) {
        ea = ea * 1.001f + 0.0005f;
    }
    float eb = -7.0f;
    for (int i = 0; i < 150; i++) {
        eb = eb / 1.002f - 0.25f;
    }
    uint32_t exp_a, exp_b;
    memcpy(&exp_a, &ea, 4U);
    memcpy(&exp_b, &eb, 4U);
    
    if (s_fpa_bits != exp_a || s_fpb_bits != exp_b) {
        fr(b, l, "FP mismatch a=%08lX/%08lX b=%08lX/%08lX",
           (unsigned long)s_fpa_bits, (unsigned long)exp_a,
           (unsigned long)s_fpb_bits, (unsigned long)exp_b);
        return TFW_FAIL;
    }
    float eh = 3.5f;
    for (int i = 0; i < 40; i++) {
        eh = eh * 1.01f;
    }
    uint32_t got_h, exp_h;
    memcpy(&got_h, &here, 4U);
    memcpy(&exp_h, &eh, 4U);
    if (got_h != exp_h) {
        fr(b, l, "caller FP context corrupted");
        return TFW_FAIL;
    }
    return TFW_PASS;
}

/* --------------------------------------------------------------- FRT-021 */
static TaskHandle_t s_low, s_high;

static void t_low(void *arg)
{
    SemaphoreHandle_t mx = (SemaphoreHandle_t)arg;

    (void)xSemaphoreTake(mx, portMAX_DELAY);
    vTaskDelay(pdMS_TO_TICKS(500)); /* holds the mutex, gets boosted */
    (void)xSemaphoreGive(mx);
    vTaskDelay(pdMS_TO_TICKS(400));
    s_low = NULL;
    vTaskDelete(NULL);
}

static void t_high(void *arg)
{
    SemaphoreHandle_t mx = (SemaphoreHandle_t)arg;

    vTaskDelay(pdMS_TO_TICKS(100));
    (void)xSemaphoreTake(mx, portMAX_DELAY); /* blocks low -> inherits */
    s_high = NULL;
    vTaskDelete(NULL);
}

static tfw_result_t test_kernel_objects(char *b, size_t l)
{
    /* --- queue: fill, overflow, drain, timeout --- */
    QueueHandle_t q = xQueueCreate(3, sizeof(uint32_t));
    uint32_t v = 0U;

    if (q == NULL) { fr(b, l, "queue create"); return TFW_FAIL; }
    for (uint32_t i = 0; i < 3U; i++) {
        if (xQueueSend(q, &i, 0) != pdPASS) {
            fr(b, l, "queue send %lu failed", (unsigned long)i);
            vQueueDelete(q);
            return TFW_FAIL;
        }
    }
    if (xQueueSend(q, &v, pdMS_TO_TICKS(50)) != errQUEUE_FULL) {
        fr(b, l, "overflow send unexpectedly accepted");
        vQueueDelete(q);
        return TFW_FAIL;
    }
    if (xQueueReceive(q, &v, 0) != pdTRUE || v != 0U) {
        fr(b, l, "queue receive wrong");
        vQueueDelete(q);
        return TFW_FAIL;
    }
    for (int i = 0; i < 2; i++) { /* drain the rest before testing the timeout */
        (void)xQueueReceive(q, &v, 0);
    }
    if (xQueueReceive(q, &v, pdMS_TO_TICKS(50)) != errQUEUE_EMPTY) {
        fr(b, l, "empty receive did not time out");
        vQueueDelete(q);
        return TFW_FAIL;
    }
    vQueueDelete(q);

    /* --- binary semaphore --- */
    SemaphoreHandle_t bs = xSemaphoreCreateBinary();
    if (bs == NULL) { fr(b, l, "binary sem create"); return TFW_FAIL; }
    if (xSemaphoreTake(bs, 0) != pdFALSE) {
        fr(b, l, "binary take succeeded before give");
        vSemaphoreDelete(bs);
        return TFW_FAIL;
    }
    (void)xSemaphoreGive(bs);
    if (xSemaphoreTake(bs, 0) != pdTRUE) {
        fr(b, l, "binary take after give failed");
        vSemaphoreDelete(bs);
        return TFW_FAIL;
    }
    vSemaphoreDelete(bs);

    /* --- counting semaphore capped at 5 --- */
    SemaphoreHandle_t cs = xSemaphoreCreateCounting(5, 0);
    if (cs == NULL) { fr(b, l, "counting sem create"); return TFW_FAIL; }
    int gave = 0;
    while (gave < 6 && xSemaphoreGive(cs) == pdTRUE) { gave++; }
    if (gave != 5) {
        fr(b, l, "counting max not enforced (gave %d)", gave);
        vSemaphoreDelete(cs);
        return TFW_FAIL;
    }
    vSemaphoreDelete(cs);

    /* --- mutex priority inheritance --- */
    SemaphoreHandle_t mx = xSemaphoreCreateMutex();
    if (mx == NULL) { fr(b, l, "mutex create"); return TFW_FAIL; }
    s_low = NULL;
    s_high = NULL;
    if (xTaskCreate(t_low, "plw", 256, (void *)mx, 2, &s_low) != pdPASS ||
        xTaskCreate(t_high, "phw", 256, (void *)mx, 7, &s_high) != pdPASS) {
        fr(b, l, "inheritance task create failed");
        vTaskDelete(s_low);
        vSemaphoreDelete(mx);
        return TFW_FAIL;
    }
    /* t_low takes the mutex immediately; t_high blocks on it at t=100ms;
     * check the boost mid-flight at t=[250..400]ms */
    vTaskDelay(pdMS_TO_TICKS(250));
    UBaseType_t lp = uxTaskPriorityGet(s_low);
    if (lp != 7U) {
        fr(b, l, "no priority inheritance (prio=%lu)", (unsigned long)lp);
        if (s_low != NULL) { vTaskDelete(s_low); }
        if (s_high != NULL) { vTaskDelete(s_high); }
        vSemaphoreDelete(mx);
        return TFW_FAIL;
    }
    vTaskDelay(pdMS_TO_TICKS(500)); /* t=750: released and restored */
    lp = (s_low != NULL) ? uxTaskPriorityGet(s_low) : 2U;
    if (lp != 2U) {
        fr(b, l, "priority not restored (prio=%lu)", (unsigned long)lp);
        if (s_low != NULL) { vTaskDelete(s_low); }
        if (s_high != NULL) { vTaskDelete(s_high); }
        vSemaphoreDelete(mx);
        return TFW_FAIL;
    }
    if (s_low != NULL) { vTaskDelete(s_low); }
    if (s_high != NULL) { vTaskDelete(s_high); }
    vSemaphoreDelete(mx);

    /* --- event group --- */
    EventGroupHandle_t eg = xEventGroupCreate();
    if (eg == NULL) { fr(b, l, "event group create"); return TFW_FAIL; }
    EventBits_t got = xEventGroupSetBits(eg, 0x5);
    if (got != 0x5) {
        fr(b, l, "event set returned %lX", (unsigned long)got);
        vEventGroupDelete(eg);
        return TFW_FAIL;
    }
    got = xEventGroupWaitBits(eg, 0x5, pdTRUE, pdTRUE, pdMS_TO_TICKS(50));
    if ((got & 0x5) != 0x5) {
        fr(b, l, "event wait lost bits");
        vEventGroupDelete(eg);
        return TFW_FAIL;
    }
    got = xEventGroupWaitBits(eg, 0x5, pdTRUE, pdTRUE, pdMS_TO_TICKS(50));
    if (got != 0U) {
        fr(b, l, "event bits not cleared");
        vEventGroupDelete(eg);
        return TFW_FAIL;
    }
    vEventGroupDelete(eg);

    /* --- task notifications --- */
    TaskHandle_t me = xTaskGetCurrentTaskHandle();
    (void)xTaskNotify(me, 1UL, eIncrement);
    (void)xTaskNotify(me, 1UL, eIncrement);
    uint32_t notif = 0U;
    if (xTaskNotifyWait(0, 0xFFFFFFFF, &notif, 0) != pdTRUE || notif != 2U) {
        fr(b, l, "notify count %lu", (unsigned long)notif);
        return TFW_FAIL;
    }

    /* --- stream buffer --- */
    StreamBufferHandle_t sb = xStreamBufferCreate(64, 1);
    if (sb == NULL) { fr(b, l, "stream buffer create"); return TFW_FAIL; }
    char txb[8] = "abc";
    char rxb[8] = {0};
    size_t n = xStreamBufferSend(sb, txb, 3, 0);
    if (n != 3U || xStreamBufferReceive(sb, rxb, 8, 0) != 3U ||
        memcmp(txb, rxb, 3) != 0) {
        fr(b, l, "stream buffer data mismatch");
        vStreamBufferDelete(sb);
        return TFW_FAIL;
    }
    vStreamBufferDelete(sb);

    /* --- message buffer --- */
    MessageBufferHandle_t mb = xMessageBufferCreate(128);
    if (mb == NULL) { fr(b, l, "message buffer create"); return TFW_FAIL; }
    char txc[5] = "wxyz";
    char rxc[8] = {0};
    (void)xMessageBufferSend(mb, txc, 5, 0);
    if (xMessageBufferReceive(mb, rxc, 8, 0) != 5U ||
        memcmp(txc, rxc, 5) != 0) {
        fr(b, l, "message buffer data mismatch");
        vMessageBufferDelete(mb);
        return TFW_FAIL;
    }
    vMessageBufferDelete(mb);
    return TFW_PASS;
}

/* --------------------------------------------------------------- FRT-022 */
static TimerHandle_t tmr_oneshot;
static TimerHandle_t tmr_reload;
static volatile uint32_t s_os_fires;
static volatile uint32_t s_rl_fires;
static volatile TickType_t s_rl_last;
static volatile uint32_t s_rl_delta_sum;

static void oneshot_cb(TimerHandle_t t)
{
    (void)t;
    s_os_fires++;
}

static void reload_cb(TimerHandle_t t)
{
    TickType_t now = xTaskGetTickCount();

    (void)t;
    if (s_rl_fires > 0U) {
        s_rl_delta_sum += (uint32_t)(now - s_rl_last);
    }
    s_rl_last = now;
    s_rl_fires++;
}

static tfw_result_t test_timers(char *b, size_t l)
{
    s_os_fires = 0U;
    s_rl_fires = 0U;
    s_rl_delta_sum = 0U;

    tmr_oneshot = xTimerCreate("os1", pdMS_TO_TICKS(400), pdFALSE, NULL,
                               oneshot_cb);
    tmr_reload  = xTimerCreate("rl1", pdMS_TO_TICKS(250), pdTRUE, NULL,
                               reload_cb);
    if (tmr_oneshot == NULL || tmr_reload == NULL) {
        fr(b, l, "timer create failed");
        return TFW_FAIL;
    }
    xTimerStart(tmr_oneshot, 0);
    xTimerStart(tmr_reload, 0);
    vTaskDelay(pdMS_TO_TICKS(1150));
    xTimerStop(tmr_oneshot, 0);
    xTimerStop(tmr_reload, 0);

    if (s_os_fires != 1U) {
        fr(b, l, "one-shot fired %lu times", (unsigned long)s_os_fires);
        xTimerDelete(tmr_oneshot, 0);
        xTimerDelete(tmr_reload, 0);
        return TFW_FAIL;
    }
    if (s_rl_fires < 3U) {
        fr(b, l, "auto-reload fired only %lu times",
           (unsigned long)s_rl_fires);
        xTimerDelete(tmr_oneshot, 0);
        xTimerDelete(tmr_reload, 0);
        return TFW_FAIL;
    }
    /* mean period within +-1 tick of 250 ms (FRT-022) */
    uint32_t mean = s_rl_delta_sum / (s_rl_fires - 1U);
    if ((int32_t)mean < (int32_t)(250U - 1U) ||
        (int32_t)mean > (int32_t)(250U + 1U)) {
        fr(b, l, "mean period %lu ms not 250+-1", (unsigned long)mean);
        xTimerDelete(tmr_oneshot, 0);
        xTimerDelete(tmr_reload, 0);
        return TFW_FAIL;
    }
    xTimerDelete(tmr_oneshot, 0);
    xTimerDelete(tmr_reload, 0);
    return TFW_PASS;
}

/* --------------------------------------------------------------- FRT-023 */
void hwval_dwt_stats_init(void); /* rtos.c — CYCCNT source */

/* TIM2 as a 32-bit 1 MHz reference against the 1 kHz SysTick tick. */
static tfw_result_t test_tick_rate(char *b, size_t l)
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    /* APB1 timer clock = PCLK1 x2 when the APB1 prescaler is not 1. */
    uint32_t timclk = ((RCC->CFGR & RCC_CFGR_PPRE1) == 0U) ? pclk1 : pclk1 * 2U;

    TIM2->CR1 = 0U;
    TIM2->PSC = (timclk / 1000000U) - 1U; /* 1 MHz */
    TIM2->ARR = 0xFFFFFFFFU;
    TIM2->EGR = TIM_EGR_UG;
    TIM2->CR1 = TIM_CR1_CEN;

    uint32_t t0 = TIM2->CNT;
    vTaskDelay(pdMS_TO_TICKS(1000));
    uint32_t t1 = TIM2->CNT;
    TIM2->CR1 = 0U;

    uint32_t elapsed = t1 - t0;
    int64_t err = (int64_t)(uint64_t)elapsed - 1000000LL;
    if (err < -1000LL || err > 1000LL) { /* +-0.1 % */
        fr(b, l, "tick drift %ld us in 1 s (counts=%lu)",
           (long)err, (unsigned long)elapsed);
        return TFW_FAIL;
    }
    return TFW_PASS;
}

/* --------------------------------------------------------------- FRT-024 */
/* IRQ latency: pend TIM2's NVIC line in software, measure cycles to handler
 * entry with DWT. Context switch: queue wake-up round trip. */
static volatile uint32_t s_irq_entry_cycles;
static volatile int s_irq_captured;
static volatile uint32_t s_sw_entry_cycles;

void TIM2_IRQHandler(void);

void TIM2_IRQHandler(void)
{
    if (s_irq_captured == 0) {
        s_irq_entry_cycles = DWT->CYCCNT;
        s_irq_captured = 1;
    }
    TIM2->SR = ~TIM_SR_UIF;
}

static void hi_task(void *arg)
{
    QueueHandle_t q = (QueueHandle_t)arg;
    uint32_t v = 0U;

    (void)xQueueReceive(q, &v, portMAX_DELAY);
    s_sw_entry_cycles = DWT->CYCCNT;
    for (;;) {
        vTaskDelay(portMAX_DELAY);
    }
}

static tfw_result_t test_latencies(char *b, size_t l)
{
    hwval_dwt_stats_init();

    /* --- interrupt latency --- */
    NVIC_SetPriority(TIM2_IRQn, 6); /* >= max syscall prio (FRT-008) */
    NVIC_EnableIRQ(TIM2_IRQn);
    s_irq_captured = 0;
    __disable_irq();
    uint32_t pre = DWT->CYCCNT;
    NVIC_SetPendingIRQ(TIM2_IRQn);
    __enable_irq();
    for (int i = 0; (i < 100000) && !s_irq_captured; i++) {
        __NOP();
    }
    NVIC_DisableIRQ(TIM2_IRQn);
    if (!s_irq_captured) {
        fr(b, l, "TIM2 pend did not fire handler");
        return TFW_FAIL;
    }
    uint32_t irq_latency = s_irq_entry_cycles - pre;

    /* --- context switch --- */
    QueueHandle_t q = xQueueCreate(1, sizeof(uint32_t));
    TaskHandle_t hi = NULL;

    if (q == NULL) {
        fr(b, l, "queue create failed");
        return TFW_FAIL;
    }
    s_sw_entry_cycles = 0U;
    if (xTaskCreate(hi_task, "hi", 256, (void *)q, 7, &hi) != pdPASS) {
        fr(b, l, "hi task create failed");
        vQueueDelete(q);
        return TFW_FAIL;
    }
    vTaskDelay(2U);
    uint32_t v = 42U;
    uint32_t pre2 = DWT->CYCCNT;
    (void)xQueueSend(q, &v, 0); /* preempts immediately into hi_task */
    uint32_t switch_cycles = s_sw_entry_cycles - pre2;
    (void)vTaskDelete(hi);
    vQueueDelete(q);

    if (irq_latency == 0U || switch_cycles == 0U) {
        fr(b, l, "latency not captured (irq=%lu sw=%lu)",
           (unsigned long)irq_latency, (unsigned long)switch_cycles);
        return TFW_FAIL;
    }
    fr(b, l, "irq-latency=%lu cyc ctx-switch=%lu cyc",
       (unsigned long)irq_latency, (unsigned long)switch_cycles);
    return TFW_PASS;
}

/* --------------------------------------------------------------- FRT-025 */
static tfw_result_t test_tasklist_stats(char *b, size_t l)
{
    UBaseType_t n = uxTaskGetNumberOfTasks();
    if (n < 4U) {
        fr(b, l, "suspicious task count %lu", (unsigned long)n);
        return TFW_FAIL;
    }

    TaskStatus_t *st = (TaskStatus_t *)pvPortMalloc(n * sizeof(TaskStatus_t));
    if (st == NULL) {
        fr(b, l, "pvPortMalloc of status failed");
        return TFW_FAIL;
    }
    uint32_t total_runtime = 0U;
    UBaseType_t got = uxTaskGetSystemState(st, n, &total_runtime);
    if (got != n) {
        fr(b, l, "uxTaskGetSystemState %lu/%lu",
           (unsigned long)got, (unsigned long)n);
        vPortFree(st);
        return TFW_FAIL;
    }
    for (UBaseType_t i = 0; i < n; i++) {
        UBaseType_t hwm = uxTaskGetStackHighWaterMark(st[i].xHandle);
        if (hwm == 0U) {
            fr(b, l, "task %s has zero stack headroom", st[i].pcTaskName);
            vPortFree(st);
            return TFW_FAIL;
        }
    }
    vPortFree(st);

    char *stats = (char *)pvPortMalloc(2048);
    if (stats == NULL) {
        fr(b, l, "stats buffer alloc failed");
        return TFW_FAIL;
    }
    vTaskGetRunTimeStats(stats);
    int ok = (strstr(stats, "cli") != NULL) && (strstr(stats, "%") != NULL);
    vPortFree(stats);
    if (!ok) {
        fr(b, l, "run-time stats empty");
        return TFW_FAIL;
    }
    return TFW_PASS;
}

/* --------------------------------------------------------------- FRT-026 */
/* Deliberate overflow / oversize allocation — the hooks end in a reset, so
 * this only runs through the CLI with destructive allowed (section 7). */
static tfw_result_t test_hooks_destructive(char *b, size_t l)
{
    (void)l;
    (void)snprintf(b, HWVAL_REASON_BUF, "destructive; run via CLI");
    return TFW_SKIP;
}

/* ----------------------------------------------------------------- table */
const struct tfw_case g_rtos_tests[] = {
    { "RTOS.020",      "RTOS", "task create/suspend/resume/prio/delete",
      TFW_LEVEL_S, 0, 0, 1500U, "FRT-020",                test_tasks },
    { "RTOS.021",      "RTOS", "queues, semaphores, events, notify, buffers",
      TFW_LEVEL_S, 0, 0, 3000U, "FRT-021",                test_kernel_objects },
    { "RTOS.021.FPU",  "RTOS", "lazy FPU context across 3 tasks",
      TFW_LEVEL_S, 0, 0, 2000U, "FRT-009",                test_fpu_lazy_stacking },
    { "RTOS.022",      "RTOS", "software timer period accuracy",
      TFW_LEVEL_S, 0, 0, 2000U, "FRT-022",                test_timers },
    { "RTOS.023",      "RTOS", "1 kHz tick vs TIM2 +-0.1 %",
      TFW_LEVEL_S, 0, 0, 2000U, "FRT-023, FRT-002",       test_tick_rate },
    { "RTOS.024",      "RTOS", "IRQ latency + context switch in cycles",
      TFW_LEVEL_S, 0, 0, 2000U, "FRT-024",                test_latencies },
    { "RTOS.025",      "RTOS", "task list, runtime stats, stack headroom",
      TFW_LEVEL_S, 0, 0, 1000U, "FRT-025",                test_tasklist_stats },
    { "RTOS.026",      "RTOS", "overflow/malloc hooks (resets MCU)",
      TFW_LEVEL_S, 0, 1, 1000U, "FRT-026, FRT-004, FRT-005",
      test_hooks_destructive },
};

const size_t g_rtos_test_count = sizeof(g_rtos_tests) / sizeof(g_rtos_tests[0]);

void hwval_test_register_rtos(void)
{
    hwval_test_register(g_rtos_tests, g_rtos_test_count);
}