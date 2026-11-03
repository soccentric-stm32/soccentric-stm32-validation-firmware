#ifndef HWVAL_SECTIONS_H
#define HWVAL_SECTIONS_H

/*
 * hwval_sections.h - linker-section attributes for the RAM budget
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * Target:  NUCLEO-F767ZI (STM32F767ZIT6)
 * SRS:     NFR-004 (>= 256 KB SRAM1 and >= 64 KB DTCM left free for the
 *          destructive RAM tests), GEN-007 (DMA buffers coherent with
 *          the Caches).
 *
 * RM0410 (2.1.6/2.1.8/2.1.13): the F767 has no MDMA (H7-only); any DMA
 * engine can still reach DTCM, but only through the CPU's AHBS slave
 * bus, which is slow. The bulk DMA-fed test buffers therefore live in
 * SRAM1 (section ".sram1", NOLOAD in the linker script). MPU region 3
 * in src/mpu_setup.c marks that span non-cacheable so CPU fills are
 * directly visible to the DMA masters (GEN-007); main.c zeroes the
 * span at boot to give the C zero-init semantics of static storage.
 */

#define HWVAL_SRAM1 __attribute__((section(".sram1"), aligned(32)))

#endif /* HWVAL_SECTIONS_H */