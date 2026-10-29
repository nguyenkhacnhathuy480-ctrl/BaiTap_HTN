#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "board_config.h"

#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configCPU_CLOCK_HZ                      BOARD_SYSTEM_CLOCK_HZ
#define configTICK_RATE_HZ                      ((TickType_t)1000U)
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                ((uint16_t)128U)
#define configTOTAL_HEAP_SIZE                   ((size_t)(6U * 1024U))
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0
#define configQUEUE_REGISTRY_SIZE               4
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  1
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configUSE_CO_ROUTINES                   0
#define configUSE_TIMERS                        0
#define configUSE_TRACE_FACILITY                0
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1

#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskGetSchedulerState          1
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_uxTaskGetStackHighWaterMark     1

#define configKERNEL_INTERRUPT_PRIORITY         (15U << 4)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    (5U << 4)
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define vPortSVCHandler                         SVC_Handler
#define xPortPendSVHandler                      PendSV_Handler
#define xPortSysTickHandler                    SysTick_Handler

#endif
