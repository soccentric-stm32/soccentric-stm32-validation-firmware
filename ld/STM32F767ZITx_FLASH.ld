/*
 * ld/STM32F767ZITx_FLASH.ld - HWVAL linker script
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * Target:  STM32F767ZIT6 (1 MB x 2 flash, DTCM 128 KB, SRAM1 368 KB,
 *          SRAM2 16 KB), NUCLEO-F767ZI
 * SRS:     GEN-006, GEN-007, TFW-022, ASM-004, NFR-004
 *
 * Flash sectors (RM0410): 0-3 = 32 KB each, 4 = 128 KB, 5-11 = 256 KB each.
 * Sector 11 (0x081C0000..0x081FFFFF) is deliberately not part of the FLASH
 * region: it stays free for the destructive flash tests (FLS-005).
 */

ENTRY(Reset_Handler)

/* Main/exception stack top: top of DTCM RAM. */
_estack = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);

MEMORY
{
    /* Sectors 0-10 only: 512 KiB * 3.5 = 1792 KiB (GEN-006, NFR-003). */
    FLASH   (rx)  : ORIGIN = 0x08000000, LENGTH = 1792K
    DTCMRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 128K
    /* SRAM1 minus the 8 KiB .noinit block at its tail. */
    SRAM1   (rwx) : ORIGIN = 0x20020000, LENGTH = 360K
    /* .noinit: survives any reset that does not remove power (TFW-022). */
    NOINIT  (rw)  : ORIGIN = 0x2007A000, LENGTH =   8K
    /* SRAM2, general use by the kernel/tasks. */
    SRAM2   (rwx) : ORIGIN = 0x2007C000, LENGTH =   8K
    /* .noncacheable: MPU non-cacheable block for DMA buffers (GEN-007). */
    NCACHE  (rw)  : ORIGIN = 0x2007E000, LENGTH =   8K
}

/* No libc heap: FreeRTOS heap_4 is static (section 5 of the SRS). */
_Min_Heap_Size  = 0;
/* MSP exception stack guard. */
_Min_Stack_Size = 0x1000;

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text .text.*)
        *(.rodata .rodata.*)
        KEEP(*(.init))
        KEEP(*(.fini))
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    .preinit_array :
    {
        PROVIDE_HIDDEN(__preinit_array_start = .);
        KEEP(*(.preinit_array*))
        PROVIDE_HIDDEN(__preinit_array_end = .);
    } > FLASH

    .init_array :
    {
        PROVIDE_HIDDEN(__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array*))
        PROVIDE_HIDDEN(__init_array_end = .);
    } > FLASH

    .fini_array :
    {
        PROVIDE_HIDDEN(__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array*))
        PROVIDE_HIDDEN(__fini_array_end = .);
    } > FLASH

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data .data.*)
        . = ALIGN(4);
        _edata = .;
    } > DTCMRAM AT > FLASH

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > DTCMRAM

    .noinit (NOLOAD) :
    {
        . = ALIGN(4);
        __noinit_start = .;
        *(.noinit .noinit.*)
        . = ALIGN(4);
        __noinit_end = .;
    } > NOINIT

    .noncacheable (NOLOAD) :
    {
        . = ALIGN(32);
        __noncacheable_start = .;
        *(.noncacheable .noncacheable.*)
        . = ALIGN(32);
        __noncacheable_end = .;
    } > NCACHE

    ._user_heap_stack (NOLOAD) :
    {
        . = ALIGN(8);
        PROVIDE(end = .);
        . = . + _Min_Heap_Size;
        . = . + _Min_Stack_Size;
        . = ALIGN(8);
    } > DTCMRAM
}

/* GEN-006: the whole image (including the .data load image) stays inside
 * sectors 0-10. The build fails otherwise. */
__flash_image_end = LOADADDR(.data) + SIZEOF(.data);
ASSERT(ORIGIN(FLASH) + LENGTH(FLASH) == 0x081C0000,
       "GEN-006: FLASH region must end exactly at the sector-11 boundary")
ASSERT(__flash_image_end <= 0x081C0000,
       "GEN-006: image overlaps flash sector 11 (reserved, ASM-004)")
ASSERT(_edata - _sdata + (_ebss - _sbss) + _Min_Stack_Size <= LENGTH(DTCMRAM),
       "DTCM overflow")