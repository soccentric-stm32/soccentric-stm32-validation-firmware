/*
 * board/stm32f7xx_hal_conf.h - project HAL configuration
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * Target:  STM32F767ZIT6, NUCLEO-F767ZI
 * SRS:     CON-002 (drivers used as shipped; this is the project-owned
 *          configuration file), GEN-036/008 (assert text via fault.c)
 */

#ifndef STM32F7xx_HAL_CONF_H
#define STM32F7xx_HAL_CONF_H

/* Oscillator values in Hz (see Hardware.md section 6; CON-005: PH0 carries
 * the ST-LINK 8 MHz MCO in bypass mode). */
#define HSE_VALUE    8000000U
#define HSE_STARTUP_TIMEOUT 100U
#define HSI_VALUE   16000000U
#define LSI_VALUE    32000U
#define LSE_VALUE    32768U
#define LSE_STARTUP_TIMEOUT 5000U
#define EXTERNAL_SAI1_CLOCK_VALUE 48000U
#define EXTERNAL_SAI2_CLOCK_VALUE 48000U
#define EXTERNAL_CLOCK_VALUE 12288000U

#define VDD_VALUE   3300U
/* legacy PHY-driver timeouts required by the (as-shipped) ETH HAL */
#ifndef PHY_READ_TO
#define PHY_READ_TO 5000U
#endif
#ifndef PHY_WRITE_TO
#define PHY_WRITE_TO 5000U
#endif
#define TICK_INT_PRIORITY 15U
#define USE_RTOS 0U
#define PREFETCH_ENABLE 1U

/* Enabled HAL modules — the list grows per SRS section (NFR-008). */
#define HAL_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_IWDG_MODULE_ENABLED
#define HAL_CRC_MODULE_ENABLED
#define HAL_ETH_MODULE_ENABLED      /* ETH-007/008 LwIP stack (section 11)   */

#include "stm32f7xx_hal_rcc.h"
#include "stm32f7xx_hal_gpio.h"
#include "stm32f7xx_hal_cortex.h"
#include "stm32f7xx_hal_pwr.h"
#include "stm32f7xx_hal_pwr_ex.h"
#include "stm32f7xx_hal_flash.h"
#include "stm32f7xx_hal_flash_ex.h"
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_iwdg.h"
#include "stm32f7xx_hal_crc.h"
#include "stm32f7xx_hal_eth.h"

/* Syscalls use the fault.c reporting path; HAL errors are not fatal by
 * default, tests assert explicitly. */
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);
#endif

/* assert_param: parameter checks compile out unless USE_FULL_ASSERT. */
#ifdef  USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
#else
  #define assert_param(expr) ((void)0U)
#endif

#endif /* STM32F7xx_HAL_CONF_H */