/*
 * board/lwip_arch/arch/sys_arch.h - minimal NO_SYS stub (ETH-007)
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-03
 * SRS:     ETH-007 (LwIP from the SDK). lwip/sys.h includes this header
 *          unconditionally even for NO_SYS=1 builds; only the SYS_ARCH
 *          critical-section macros are actually used in that mode.
 */

#ifndef __HWVAL_LWIP_SYS_ARCH_H__
#define __HWVAL_LWIP_SYS_ARCH_H__

#include "lwip/opt.h"

#if !NO_SYS
#error "HWVAL builds LwIP in polled NO_SYS mode only (ETH-007/008)"
#endif

#include <stdint.h>

typedef uint32_t sys_sem_t;
typedef uint32_t sys_mutex_t;
typedef uint32_t sys_mbox_t;
typedef uint32_t sys_thread_t;
typedef void    *sys_mbox_msg_t;
typedef uint32_t sys_prot_t;    /* implemented in src/net_lwip.c: the
                                 * SYS_LIGHTWEIGHT_PROT macros of lwip/sys.h
                                 * route to sys_arch_protect/unprotect */

#endif /* __HWVAL_LWIP_SYS_ARCH_H__ */