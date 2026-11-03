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
#include "FreeRTOS.h"
#include "task.h"

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

    NVIC_SetPriority(USART3_IRQn, 6); /* >= configMAX_SYSCALL (FRT-008) */
    NVIC_EnableIRQ(USART3_IRQn);

    USART3->CR1 = 0U; /* disabled while configuring */
    USART3->BRR = (HAL_RCC_GetPCLK1Freq() + (HWVAL_VCP_BAUD / 2U)) /
                   HWVAL_VCP_BAUD;
    USART3->CR2 = 0U; /* 1 stop bit */
    USART3->CR3 = 0U; /* no flow control; driver DMA lands in section 6 */
    USART3->RQR = USART_RQR_RXFRQ; /* drop any stale bytes */
    USART3->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE |
                  USART_CR1_UE;    /* interrupt-driven RX (CLI-002) */

    s_ready = 1;
}

/* --- RX ring buffer (CLI-002: >= 512 bytes, no loss on paste) ------------- */
#define RX_RING_SIZE 512U
static volatile uint8_t  s_rx_ring[RX_RING_SIZE];
static volatile uint16_t s_rx_head; /* ISR writes */
static volatile uint16_t s_rx_tail; /* task reads */

void USART3_IRQHandler(void)
{
    uint32_t isr = USART3->ISR;

    if ((isr & USART_ISR_RXNE) != 0U) {
        uint16_t next = (uint16_t)((s_rx_head + 1U) % RX_RING_SIZE);
        uint8_t  data = (uint8_t)USART3->RDR;

        s_rx_ring[s_rx_head] = data;
        if (next != s_rx_tail) {
            s_rx_head = next; /* drop newest on overflow, flags in section 6 */
        }
    }
    if ((isr & USART_ISR_ORE) != 0U) {
        USART3->ICR = USART_ICR_ORECF; /* overrun: recover, reader drains */
    }
}

int hwval_uart3_getc(void)
{
    uint16_t head;
    uint8_t  c;

    taskENTER_CRITICAL();
    {
        head = s_rx_head;
        if (head == s_rx_tail) {
            taskEXIT_CRITICAL();
            return -1;
        }
        c = s_rx_ring[s_rx_tail];
        s_rx_tail = (uint16_t)((s_rx_tail + 1U) % RX_RING_SIZE);
    }
    taskEXIT_CRITICAL();
    return (int)c;
}

void hwval_uart3_rx_wakeup(void)
{
    /* Section 6 gives a semaphore here on RXNE; section 5 polls from the
     * CLI task instead. */
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