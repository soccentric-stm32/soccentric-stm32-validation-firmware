/*
 * board/lwipopts.h - LwIP configuration for the HWVAL network stack
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-03
 * SRS:     ETH-007 (LwIP from the SDK, DHCP + static fallback, ICMP,
 *          TCP/UDP echo), ETH-008 (TCP throughput echo target).
 *
 * NO_SYS = 1: LwIP runs polled from the Runner task context (the cases
 * call net_lwip_poll()), single-context, so SYS_ARCH critical sections
 * resolve to BASEPRI sections (safe and cheap).
 *
 * RAM budget interplay (NFR-004: >= 256 KB SRAM1 + >= 64 KB DTCM free):
 * - the ETH DMA buffers live in .sram1 (HWVAL_SRAM1, non-cacheable,
 *   src/net_lwip.c) and the descriptor tables in .noncacheable;
 * - LwIP's own small pools (MEM 4 KB + PBUF pool) stay CPU-side in the
 *   DTCM .bss; sizes are tuned so the DTCM floor keeps ~2 KB margin.
 */

#ifndef HWVAL_LWIPOPTS_H
#define HWVAL_LWIPOPTS_H

/* ---------- OS ---------- */
#define NO_SYS                      1
#define SYS_LIGHTWEIGHT_PROT        1

/* ---------- memory ---------- */
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    2048

#define MEMP_NUM_PBUF               4
#define MEMP_NUM_UDP_PCB            2
#define MEMP_NUM_TCP_PCB            4
#define MEMP_NUM_TCP_PCB_LISTEN     2
#define MEMP_NUM_TCP_SEG            24
#define MEMP_NUM_SYS_TIMEOUT        6

/* ---------- pbuf ---------- */
#define PBUF_POOL_SIZE              3
#define PBUF_POOL_BUFSIZE           1524
#define ETH_PAD_SIZE                2

/* ---------- IPv4 / protocols ---------- */
#define LWIP_IPV4                   1
#define LWIP_IPV6                   0
#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_IGMP                   0
#define LWIP_AUTOIP                 0
#define LWIP_DHCP                   1
#define DHCP_DOES_ARP_CHECK         0   /* no ARP probe before claiming */
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_RAW                    1
#define LWIP_DNS                    0
#define LWIP_NETIF_LOOPBACK         1   /* ETH-008 in-chip throughput leg */
#define LWIP_CHECKSUM_ON_COPY       1

/* ---------- TCP ---------- */
#define TCP_TTL                     64
#define TCP_MSS                     (1500 - 40)
#define TCP_SND_BUF                 (2 * TCP_MSS)
#define TCP_WND                     (1 * TCP_MSS)
#define TCP_QUEUE_OOSEQ             0
#define TCP_MSL                     5000UL
#define LWIP_TCP_KEEPALIVE          0

/* ---------- misc ---------- */
#define LWIP_STATS                  0
#define LWIP_NETIF_STATUS_CALLBACK  0
#define LWIP_NETIF_LINK_CALLBACK    0
#define LWIP_NETCONN                0
#define LWIP_SOCKET                 0
#define LWIP_PROVIDE_ERRNO          0
#define LWIP_DEBUG                  0

#endif /* HWVAL_LWIPOPTS_H */