/*
 * harness.c - loopback harness wire detection (Appendix A, ENV-002)
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * SRS:     ENV-002, CLI-030, GPIO-004.
 *
 * The pin pairs of Appendix A are fixed against [DS11532]/[UM1974] during
 * the section 8.13 (GPIO) pass (CL-005); until then the table is empty and
 * `harness` reports zero fitted wires. Detection drives one end and reads
 * the other, for each pair.
 */

#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "hwval_board.h"
#include "harness.h"
#include "uart3.h"

struct harness_pair {
    GPIO_TypeDef *pa; uint32_t pin_a;
    GPIO_TypeDef *pb; uint32_t pin_b;
    const char   *name;
    const char   *used_by;
};

/* CL-005: pin pairs land here with the section 8.13 pass. */
static const struct harness_pair k_pairs[] = { /* empty for now */ };

int hwval_harness_count(void)
{
    return (int)(sizeof(k_pairs) / sizeof(k_pairs[0]));
}

/* Detect one wire: drive A low/high, B is input with pull-up; B follows A
 * in both directions -> wire fitted. Returns 1/0. */
static int pair_detect(const struct harness_pair *p)
{
    GPIO_InitTypeDef gi = {
        .Pin = p->pin_b, .Mode = GPIO_MODE_INPUT, .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_LOW
    };
    GPIO_InitTypeDef go = {
        .Pin = p->pin_a, .Mode = GPIO_MODE_OUTPUT_OD, .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW
    };

    HAL_GPIO_Init(p->pb, &gi);
    HAL_GPIO_Init(p->pa, &go);

    GPIO_PinState follow = GPIO_PIN_SET;
    HAL_GPIO_WritePin(p->pa, p->pin_a, GPIO_PIN_RESET);
    for (volatile int i = 0; i < 200; i++) { ; }
    if (HAL_GPIO_ReadPin(p->pb, p->pin_b) == GPIO_PIN_SET) {
        goto out;
    }
    HAL_GPIO_WritePin(p->pa, p->pin_a, GPIO_PIN_SET);
    for (volatile int i = 0; i < 200; i++) { ; }
    follow = HAL_GPIO_ReadPin(p->pb, p->pin_b);

out:
    /* restore: both inputs, no pull */
    gi.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(p->pb, &gi);
    HAL_GPIO_Init(p->pa, &gi);
    return (int)(follow == GPIO_PIN_SET);
}

/* Bitmask of detected wires (index i -> bit i), for ENV-002 gating. */
uint32_t hwval_harness_detected(void)
{
    uint32_t mask = 0U;

    if (!hwval_uart3_ready()) {
        return 0U;
    }
    for (unsigned i = 0U; i < (unsigned)hwval_harness_count(); i++) {
        if (pair_detect(&k_pairs[i]) != 0) {
            mask |= 1U << i;
        }
    }
    return mask;
}

int hwval_harness_fitted(int index)
{
    return (int)((hwval_harness_detected() >> index) & 1U);
}

void hwval_harness_report(void)
{
    (void)printf("harness: %d pair(s) configured, mask 0x%08lX\r\n",
                 hwval_harness_count(),
                 (unsigned long)hwval_harness_detected());
}