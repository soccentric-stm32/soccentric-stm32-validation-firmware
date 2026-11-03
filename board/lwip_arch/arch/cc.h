/*
 * board/lwip_arch/arch/cc.h - LwIP host-architecture layer (NO_SYS)
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-03
 * SRS:     ETH-007/ETH-008 (LwIP from the SDK; CON-002 as-shipped).
 *
 * This is the project-owned cc.h required by lwip/arch.h. Diagnostics
 * go through the VCP console printf; assertions report through the
 * fault recorder and stop in place (the IWDG/GEN-009 backstop then
 * recovers the board).
 */

#ifndef __HWVAL_LWIP_CC_H__
#define __HWVAL_LWIP_CC_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* byte order of the Cortex-M7 (little endian, DS11532) */
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#define BIG_ENDIAN    4321
#endif
#ifndef BYTE_ORDER
#define BYTE_ORDER LITTLE_ENDIAN
#endif

/* packed struct handling (GCC) */
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_STRUCT __attribute__((__packed__))
#define PACK_STRUCT_END
#define PACK_STRUCT_FIELD(x) x

/* diagnostics: routed to the console; arguments are unused when the
 * debug level hides them, so keep a no-arg fallback. */
#if LWIP_DEBUG
#define LWIP_PLATFORM_DIAG(x) printf x
#else
#define LWIP_PLATFORM_DIAG(x) do { } while (0)
#endif

#define LWIP_PLATFORM_ASSERT(x) do {                                     \
    printf("LWIPASSERT %s:%d: %s\r\n", __FILE__, (int)__LINE__, (char *)(x)); \
    for (;;) { ; }                                                        \
} while (0)

/* memory declaration helper: all lwIP declared pool/heap memory keeps
 * its .bss placement (CPU-only accesses, DTCM). */
#define LWIP_DECLARE_MEMORY_ALIGNED(variable_name, size) \
    uint8_t variable_name[LWIP_MEM_ALIGN_BUFFER(size)]
#define LWIP_MEM_ALIGN_BUFFER(size) (((size) + MEM_ALIGNMENT - 1U) & \
                                     ~(MEM_ALIGNMENT - 1U))

typedef uint32_t sys_prot_t;

#endif /* __HWVAL_LWIP_CC_H__ */