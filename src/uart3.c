/*
 * uart3.c - minimal polled USART3 console (boot banner until section 6)
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * Target:  STM32F767ZIT6, ST-LINK VCP (USART3 PD8 TX, PD9 RX)
 * SRS:     CLI-001, CLI-005; section 6 (CLI) will replace this with the
 *          IRQ/DMA ring-buffer driver and keep the same print helpers.
 */

#include "stm32f7xx_hal.h"
#include "hwval_board.h"
#include "uart3.h"

static int s_ready;

void hwval_uart3_init(void)
{
    GPIO_InitTypeDef io = {0};

    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_USART3_CLK_ENABLE();

    io.Pin       = GPIO_PIN_8 | GPIO_PIN_9; /* PD8 TX, PD9 RX */
    io.Mode      = GPIO_MODE_AF_PP;
    io.Pull      = GPIO_NOPULL;
    io.Speed     = GPIO_SPEED_FREQ_HIGH;
    io.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &io);

    USART3->CR1 = 0U; /* disabled while configuring */
    USART3->BRR = (HAL_RCC_GetPCLK1Freq() + (HWVAL_VCP_BAUD / 2U)) /
                   HWVAL_VCP_BAUD;
    USART3->CR2 = 0U; /* 1 stop bit */
    USART3->CR3 = 0U; /* no flow control */
    USART3->CR1 = USART_CR1_TE | USART_CR1_UE; /* TX first, 8 data bits */

    s_ready = 1;
}

int hwval_uart3_ready(void) { return s_ready; }

void hwval_uart3_write(const void *buf, size_t n)
{
    const char *p = (const char *)buf;

    if (!s_ready) {
        return;
    }
    if (n == 0U) {
        while (*p != '\0') { p++; }
        n = (size_t)(p - (const char *)buf);
        p = (const char *)buf;
    }
    while (n-- > 0U) {
        char c = *p++;
        if (c == '\n') {
            while (!(USART3->ISR & USART_ISR_TXE)) { ; }
            USART3->TDR = (uint8_t)'\r';
        }
        while (!(USART3->ISR & USART_ISR_TXE)) { ; }
        USART3->TDR = (uint8_t)c;
    }
}

void hwval_uart3_puts(const char *s)
{
    hwval_uart3_write(s, 0U);
    hwval_uart3_write("\r\n", 2U);
}