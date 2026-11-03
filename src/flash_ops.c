/*
 * flash_ops.c - flash wrapper: sector-11 only, PSIZE x32, IRQ counting
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * Target:  STM32F767ZIT6
 * SRS:     SAFE-004, FLS-005..FLS-009. Register-level (PSIZE x32 for
 *          VDDA 3.3 V per [RM0410]; HAL does not expose PSIZE directly).
 */

#include <string.h>
#include "stm32f7xx_hal.h"
#include "hwval_board.h"
#include "flash_ops.h"
#include "test_core.h"

#define FLASH_SEC11_BASE HWVAL_SECTOR11_BASE
#define FLASH_SEC11_LEN  (HWVAL_FLASH_END - HWVAL_SECTOR11_BASE)

static volatile uint32_t s_eop_count;
static volatile uint32_t s_err_count;
static volatile uint32_t s_last_err;

int hwval_flash_range_in_sector11(uint32_t addr, uint32_t len)
{
    return (addr >= FLASH_SEC11_BASE) &&
           (len <= FLASH_SEC11_LEN) &&
           (addr + len <= HWVAL_FLASH_END);
}

void hwval_flash_count_get(uint32_t *eop, uint32_t *err)
{
    *eop = s_eop_count;
    *err = s_err_count;
}

/* FLASH_IRQHandler: count EOP/errors, clear flags (FLS-008). HAL_FLASH_
 * irq.c is not compiled, so this override owns the vector. */
void FLASH_IRQHandler(void)
{
    uint32_t sr = FLASH->SR;

    if ((sr & FLASH_SR_EOP) != 0U) {
        s_eop_count++;
        SET_BIT(FLASH->SR, FLASH_SR_EOP); /* W1C */
    }
    if ((sr & (FLASH_SR_PGPERR | FLASH_SR_PGAERR | FLASH_SR_ERSERR |
               FLASH_SR_WRPERR | FLASH_SR_OPERR)) != 0U) {
        s_err_count++;
        s_last_err = sr;
        SET_BIT(FLASH->SR, sr & (FLASH_SR_PGPERR | FLASH_SR_PGAERR |
                                 FLASH_SR_ERSERR | FLASH_SR_WRPERR |
                                 FLASH_SR_OPERR));
    }
}

static int flash_wait_done(uint32_t timeout_loops)
{
    while ((FLASH->SR & FLASH_SR_BSY) != 0U) {
        if (--timeout_loops == 0U) {
            return -1;
        }
    }
    return 0;
}

static void flash_psize_x32(void)
{
    MODIFY_REG(FLASH->CR, FLASH_CR_PSIZE, FLASH_PSIZE_WORD);
}

int hwval_flash_erase_sector11(void)
{
    /* unlock */
    if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U) {
        FLASH->KEYR = 0x45670123U;
        FLASH->KEYR = 0xCDEF89ABU;
    }
    if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U) {
        return -2; /* unlock failed */
    }

    __HAL_FLASH_ENABLE_IT(FLASH_IT_EOP | FLASH_IT_ERR);
    NVIC_SetPriority(FLASH_IRQn, 6);
    NVIC_EnableIRQ(FLASH_IRQn);

    FLASH->CR = (FLASH->CR & ~FLASH_CR_SNB) |
                FLASH_CR_SER | (FLASH_SECTOR_11 << FLASH_CR_SNB_Pos);
    FLASH->CR |= FLASH_CR_STRT;

    if (flash_wait_done(20000000U) != 0) {
        return -3; /* erase timeout */
    }
    __HAL_FLASH_DISABLE_IT(FLASH_IT_EOP | FLASH_IT_ERR);

    /* lock again */
    SET_BIT(FLASH->CR, FLASH_CR_LOCK);
    return 0;
}

int hwval_flash_program_sector11(uint32_t addr, const uint32_t *data,
                                 uint32_t words)
{
    if (!hwval_flash_range_in_sector11(addr, words * 4U)) {
        return -1; /* SAFE-004: address check rejects anything else */
    }
    if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U) {
        FLASH->KEYR = 0x45670123U;
        FLASH->KEYR = 0xCDEF89ABU;
    }
    if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U) {
        return -2;
    }
    flash_psize_x32();
    __HAL_FLASH_ENABLE_IT(FLASH_IT_EOP | FLASH_IT_ERR);

    for (uint32_t i = 0U; i < words; i++) {
        *(volatile uint32_t *)addr = data[i];
        __DSB();
        if (flash_wait_done(2000000U) != 0) {
            __HAL_FLASH_DISABLE_IT(FLASH_IT_EOP | FLASH_IT_ERR);
            SET_BIT(FLASH->CR, FLASH_CR_LOCK);
            return -3;
        }
        addr += 4U;
    }
    __HAL_FLASH_DISABLE_IT(FLASH_IT_EOP | FLASH_IT_ERR);
    SET_BIT(FLASH->CR, FLASH_CR_LOCK);
    return 0;
}

int hwval_flash_program_refuse(uint32_t addr, uint32_t value)
{
    int rc = 0;

    if (READ_BIT(FLASH->CR, FLASH_CR_LOCK) != 0U) {
        FLASH->KEYR = 0x45670123U;
        FLASH->KEYR = 0xCDEF89ABU;
    }
    flash_psize_x32();
    s_err_count = 0U;
    s_last_err  = 0U;
    *(volatile uint32_t *)addr = value;
    __DSB();
    if (flash_wait_done(2000000U) != 0) {
        rc = -3;
    }
    SET_BIT(FLASH->SR, FLASH_SR_PGPERR | FLASH_SR_PGAERR |
             FLASH_SR_ERSERR | FLASH_SR_WRPERR | FLASH_SR_OPERR);
    SET_BIT(FLASH->CR, FLASH_CR_LOCK);
    return rc;
}

/* FLS-007 helper: wrong-key lock (used only as the last flash test) */
void hwval_flash_wrong_key_lock(void)
{
    FLASH->KEYR = 0x12345678U; /* first wrong key: sequence aborted */
    FLASH->KEYR = 0x87654321U;
}