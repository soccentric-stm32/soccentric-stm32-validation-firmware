/*
 * board/FreeRTOSConfig.h - project-owned FreeRTOS configuration
 *
 * Author:  Sandesh Ghimire <sandesh@soccentric.com>
 * Date:    2026-11-02
 * SRS:     FRT-001..FRT-009; FreeRTOS V10.2.x shipped in the image (CL-011)
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

/* CL-011: the SRS says "FreeRTOS V10.2.1", the SDK (CubeF7 v1.17.4) ships
 * V10.2.0 as-shipped (CON-002 forbids vendor-file changes). The version
 * macro stays true to what actually runs. */
#define HWVAL_FREERTOS_VERSION "V10.2.0"

/* --- scheduler ----------------------------------------------------------- */
#define configUSE_PREEMPTION                    1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      216000000UL /* GEN-004 */
#define configTICK_RATE_HZ                      ((TickType_t)1000) /* FRT-002 */
#define configMAX_PRIORITIES                    12
#define configMINIMAL_STACK_SIZE                ((uint16_t)256)
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TIME_SLICING                  1

/* --- memory ------------------------------------------------------------- */
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configTOTAL_HEAP_SIZE                   HWVAL_RTOS_HEAP_SIZE /* FRT-006 */
#define configAPPLICATION_ALLOCATED_HEAP        0

/* --- kernel objects (FRT-021) -------------------------------------------- */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_TASK_NOTIFICATIONS            1
#define configUSE_EVENT_GROUPS                  1

/* --- timers -------------------------------------------------------------- */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               2
#define configTIMER_QUEUE_LENGTH                8
#define configTIMER_TASK_STACK_DEPTH            256

/* --- hooks (FRT-003..FRT-005) --------------------------------------------- */
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0

/* --- tracing/stats (FRT-007, FRT-025) ------------------------------------- */
#define configUSE_TRACE_FACILITY                1
#define configGENERATE_RUN_TIME_STATS           1
#define configUSE_STATS_FORMATTING_FUNCTIONS    1
extern void hwval_dwt_stats_init(void);
extern uint32_t hwval_dwt_now(void);
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() hwval_dwt_stats_init()
#define portGET_RUN_TIME_COUNTER_VALUE()         hwval_dwt_now()

/* --- stack validation ----------------------------------------------------- */
#define configRECORD_STACK_HIGH_ADDRESS         1
#define configSTACK_DEPTH_TYPE                  uint16_t

/* --- interrupt priorities (FRT-008) ---------------------------------------- */
/* 4 priority bits on the F767 (NVIC_PRIO_BITS = 4). Kernel and every ISR
 * that uses a FromISR API must run at a priority number >= 5 (less urgent).
 * Enforced for every ISIR in HWVAL: TIM6 = 15, USART3 = 6 (section 6),
 * test ISRs are configured in their tests. */
#define HWVAL_NVIC_PRIO_BITS                    4U
#define configPRIO_BITS                         HWVAL_NVIC_PRIO_BITS
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* --- debug (FRT-003) -------------------------------------------------------- */
extern void hwval_fault_assert(const char *file, int line);
#define configASSERT(x) if ((x) == 0) { hwval_fault_assert(__FILE__, __LINE__); }

/* --- port mapping (CM7 GCC port, FRT-001) ----------------------------------- */
#define vPortSVCHandler    SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
/* FRT-009: Cortex-M7 with lazy FP stacking; the port saves the FPU context
 * for tasks that use it (verified numerically by the RTOS.021.FPU test). */

/* --- optional API includes --------------------------------------------------- */
#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_xTaskGetSchedulerState      1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_xTaskGetIdleTaskHandle      1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_uxTaskGetSystemState        1
#define INCLUDE_xTimerPendFunctionCall      1
#define INCLUDE_eTaskGetState               1

#endif /* FREERTOS_CONFIG_H */