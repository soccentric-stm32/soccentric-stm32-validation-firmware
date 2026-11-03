/*
 * hwval_limits.h - named measured limits with their sources (TFW-008)
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * Sources: [DS11532] electrical characteristics, [RM0410] clock/flash
 *          chapters, Hardware.md measured values, SRS sections.
 * Every numeric pass/fail limit used by tests is defined here.
 */

#ifndef HWVAL_LIMITS_H
#define HWVAL_LIMITS_H

/* --- clock limits (RM0410 / DS11532 tables) ------------------------------- */
#define HWVAL_LIM_SYSCLK_MAX_HZ     216000000U /* scale 1 + OD, rev Z      */
#define HWVAL_LIM_HSI_TOL_PPM       10000U     /* CLK-001: +-1 % at room   */
#define HWVAL_LIM_HSEBYP_TOL_PPM    1000U      /* CLK-002: +-0.1 %         */
#define HWVAL_LIM_LSE_TOL_PPM       100U       /* CLK-004: +-100 ppm       */
#define HWVAL_LIM_LSI_MIN_HZ        17000U     /* CLK-005: LSI range       */
#define HWVAL_LIM_LSI_MAX_HZ        47000U
#define HWVAL_LIM_RTCACC_PPM        100U       /* RTC-001 window           */

/* --- supply (PWR-001/002) -------------------------------------------------- */
#define HWVAL_LIM_VDDA_MV           3300U      /* nominal                  */
#define HWVAL_LIM_VDDA_TOL_MV       165U       /* +-5 %                    */

/* --- ADC (ADC-001/ADC-010, DS11532 5.19/5.21 ------------------------------ */
#define HWVAL_LIM_ADC_INL_LSB       5          /* 12-bit total INL         */
#define HWVAL_LIM_ADC_DNL_LSB       1
#define HWVAL_LIM_VREFINT_TOL_PCT   2          /* vs factory calibration   */
#define HWVAL_LIM_TSENSOR_MIN_C     0          /* ADC-002 bench window     */
#define HWVAL_LIM_TSENSOR_MAX_C     60
#define HWVAL_LIM_ADC_NOISE_LSB     2          /* ADC-010 std dev limit    */

/* --- flash timings (DS11532 flash characteristics tables) ------------------ */
#define HWVAL_LIM_SECTOR_ERASE_MAX_MS 2000     /* FLS-009, 256 KB sector   */
#define HWVAL_LIM_PROG64_MAX_US     1200       /* FLS-009, 64-bit word     */

/* --- watchdog (WDG-001) ----------------------------------------------------- */
#define HWVAL_LIM_LSI_WDG_TOL_PCT   60         /* 17-47 kHz spread on 32   */

/* --- RAM (RAM-001..RAM-007) ------------------------------------------------- */
#define HWVAL_LIM_RAM_MARCH_PAT     0xA5A5A5A5U /* March C- element        */

/* --- UART/SPI timing (UART-001, SPI-001) -------------------------------------
 * Baud error budget: |actual-target| / target <= 2.5 % with 16x oversample */
#define HWVAL_LIM_BAUD_TOL_PCT      3

/* --- context switching, section 5 self-tests (measured values, no limit
 *      beyond "captures a plausible non-zero latency", FRT-024) ---------------
 *      irq-latency observed 33 cycles, ctx switch 708 cycles (this build) */

#endif /* HWVAL_LIMITS_H */