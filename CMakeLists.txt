#
# CMakeLists.txt - HWVAL firmware, NUCLEO-F767ZI
#
# Author:  Sandesh Ghimire <sandesh@soccentric.com>
# Date:    2026-11-02
# Target:  STM32F767ZIT6
# SRS:     SRS-HWVAL-F767 (GEN-001, GEN-006, NFR-007, NFR-008)
#
# Build (inside the stm32-dev container, CON-001):
#   cmake -S . -B build -G Ninja
#   cmake --build build
# or simply: scripts/build.sh [clean]
#
# Outputs in build/: hwval.elf, hwval.bin, hwval.hex, hwval.map (GEN-001).
#

cmake_minimum_required(VERSION 3.22)

set(HWVAL_VERSION "0.1.0")

# Cross toolchain: the build runs in the container where arm-none-eabi-gcc
# is the only compiler. Set explicitly so a stray host compiler is never used.
set(CMAKE_SYSTEM_NAME     Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY) # no OS to run a test exe in
find_program(ARMCC arm-none-eabi-gcc REQUIRED)
find_program(ARMOBJCOPY arm-none-eabi-objcopy REQUIRED)
find_program(SIZE_TOOL arm-none-eabi-size REQUIRED)
set(CMAKE_C_COMPILER   ${ARMCC})
set(CMAKE_ASM_COMPILER ${ARMCC})
set(CMAKE_OBJCOPY      ${ARMOBJCOPY})

project(hwval LANGUAGES C ASM)

# --- SDK location ----------------------------------------------------------
# STM32CubeF7 v1.17.4 is provided by the docker image (CON-002).
set(CUBE_ROOT "$ENV{STM32CUBE_F7}")
if(NOT CUBE_ROOT)
    set(CUBE_ROOT "/opt/st/STM32CubeF7")
endif()
if(NOT EXISTS "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Inc/stm32f7xx_hal.h")
    message(FATAL_ERROR
        "STM32CubeF7 not found at ${CUBE_ROOT}.\n"
        "Build through scripts/docker-dev.sh (see docker/README.md, CL-002).")
endif()

# --- Build metadata (GEN-003) ----------------------------------------------
execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} rev-parse --short=12 HEAD
    OUTPUT_VARIABLE HWVAL_GIT_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
execute_process(COMMAND git -C ${CMAKE_CURRENT_SOURCE_DIR} log -1 --date=iso --format=%cd
    OUTPUT_VARIABLE HWVAL_COMMIT_DATE OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
string(TIMESTAMP HWVAL_BUILD_DATE "%Y-%m-%d %H:%M:%S")

# --- Toolchain flags --------------------------------------------------------
set(CPU_FLAGS -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(${CPU_FLAGS} -Wall -Wextra -Werror -ffunction-sections -fdata-sections)
add_link_options(${CPU_FLAGS} -Wl,--gc-sections -Wl,--cref
    -Wl,-Map=${CMAKE_BINARY_DIR}/hwval.map
    --specs=nano.specs -u _printf_float)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_C_FLAGS_RELEASE "-O2 -g3")
set(CMAKE_C_FLAGS_DEBUG   "-O0 -g3")

add_compile_definitions(
    STM32F767xx
    USE_HAL_DRIVER
    HSE_VALUE=8000000U          # ST-LINK MCO feeds PH0 in bypass mode (CON-005)
    HWVAL_VERSION_STR="${HWVAL_VERSION}"
    HWVAL_SRS_ID="SRS-HWVAL-F767"
    HWVAL_GIT_COMMIT="${HWVAL_GIT_COMMIT}"
    HWVAL_BUILD_DATE="${HWVAL_BUILD_DATE} ${HWVAL_COMMIT_DATE}")

# FreeRTOS heap size, configurable at build time (FRT-006).
set(HWVAL_HEAP_KB "32" CACHE STRING "FreeRTOS heap_4 size in KB (FRT-006)")
math(EXPR HWVAL_HEAP_BYTES "${HWVAL_HEAP_KB} * 1024")

include_directories(src board tests
    "${CUBE_ROOT}/Drivers/CMSIS/Include"
    "${CUBE_ROOT}/Drivers/CMSIS/Device/ST/STM32F7xx/Include"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Inc")

# --- Sources ----------------------------------------------------------------
# HAL modules used so far. The list grows section by section (NFR-008):
set(HAL_SOURCES
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_flash_ex.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_dma.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_iwdg.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_crc.c"
    "${CUBE_ROOT}/Drivers/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_crc_ex.c")

set(PROJECT_SOURCES
    src/main.c
    src/clock.c
    src/version.c
    src/uart3.c
    src/fault.c
    src/iwdg.c
    src/reset_cause.c
    src/mpu_setup.c
    src/hal_timebase.c
    src/syscalls.c
    src/rtos.c
    src/test_core.c
    src/cli.c
    src/settings.c
    src/harness.c
    tests/test_rtos.c
    tests/test_dev.c
    tests/test_core_m7.c
    tests/test_cache_mpu.c
    tests/test_ram.c
    tests/test_flash.c)
list(APPEND PROJECT_SOURCES src/flash_ops.c)
list(APPEND PROJECT_SOURCES   tests/test_clk.c)
list(APPEND PROJECT_SOURCES   tests/test_pwr.c)

set(DEVICE_SOURCES
    "${CUBE_ROOT}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c"
    "${CUBE_ROOT}/Drivers/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f767xx.s")

# FreeRTOS V10.2.x kernel + native GCC CM7 port, no CMSIS wrappers (FRT-001)
set(FREERTOS_ROOT "${CUBE_ROOT}/Middlewares/Third_Party/FreeRTOS/Source")
include_directories(
    "${FREERTOS_ROOT}/include"
    "${FREERTOS_ROOT}/portable/GCC/ARM_CM7/r0p1")
set(FREERTOS_SOURCES
    "${FREERTOS_ROOT}/list.c"
    "${FREERTOS_ROOT}/queue.c"
    "${FREERTOS_ROOT}/tasks.c"
    "${FREERTOS_ROOT}/timers.c"
    "${FREERTOS_ROOT}/event_groups.c"
    "${FREERTOS_ROOT}/stream_buffer.c"
    "${FREERTOS_ROOT}/portable/GCC/ARM_CM7/r0p1/port.c"
    "${FREERTOS_ROOT}/portable/MemMang/heap_4.c")

add_executable(hwval.elf ${PROJECT_SOURCES} ${HAL_SOURCES} ${FREERTOS_SOURCES} ${DEVICE_SOURCES})
target_compile_definitions(hwval.elf PRIVATE
    HWVAL_RTOS_HEAP_SIZE=${HWVAL_HEAP_BYTES})

# GEN-006: link code and constants into flash sectors 0-10 only; the linker
# script (ld/STM32F767ZITx_FLASH.ld) fails the build on any overlap with
# sector 11 (0x081C0000, kept free for the destructive flash tests ASM-004).
target_link_options(hwval.elf PRIVATE
    "-T${CMAKE_SOURCE_DIR}/ld/STM32F767ZITx_FLASH.ld")

set(ELF "$<TARGET_FILE:hwval.elf>")
add_custom_command(TARGET hwval.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary  ${ELF} ${CMAKE_BINARY_DIR}/hwval.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex    ${ELF} ${CMAKE_BINARY_DIR}/hwval.hex
    COMMAND ${SIZE_TOOL} $<TARGET_FILE:hwval.elf>
    BYPRODUCTS ${CMAKE_BINARY_DIR}/hwval.bin ${CMAKE_BINARY_DIR}/hwval.hex
    COMMENT "Generating hwval.bin and hwval.hex")