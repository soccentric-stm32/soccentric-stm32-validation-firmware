/*
 * uart3.h - minimal polled USART3 console
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * SRS:     CLI-001 (USART3 PD8/PD9, 115200 8N1 on the ST-LINK VCP);
 *          the interrupt/DMA ring-buffer CLI driver lands in section 6.
 */

#ifndef HWVAL_UART3_H
#define HWVAL_UART3_H

#include <stddef.h>
#include <stdint.h>

/* Init after the clock tree is up (baud is computed from PCLK1). */
void hwval_uart3_init(void);

/* true once init has run; fault handlers check before printing. */
int hwval_uart3_ready(void);

/* Send n bytes, or the whole NUL-terminated string when n == 0. */
void hwval_uart3_write(const void *buf, size_t n);

/* One line, \n expanded to \r\n (CLI-005). */
void hwval_uart3_puts(const char *s);

/* RX: pop one byte from the ring buffer, -1 if empty (CLI-002). */
int hwval_uart3_getc(void);
/* RX notify hook (section 6 replaces polling with a semaphore). */
void hwval_uart3_rx_wakeup(void);

#endif /* HWVAL_UART3_H */