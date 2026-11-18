#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H
#include <stdint.h>
#define configUSE_PREEMPTION 1
#define configCPU_CLOCK_HZ 72000000UL
#define configTICK_RATE_HZ 1000U
#define configMAX_PRIORITIES 5U
#define configMINIMAL_STACK_SIZE 128U
#define configTOTAL_HEAP_SIZE (8U * 1024U)
#define configMAX_TASK_NAME_LEN 12U
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 1
#define configUSE_TIME_SLICING 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configSUPPORT_STATIC_ALLOCATION 0
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configUSE_MUTEXES 0
#define configUSE_CO_ROUTINES 0
#define configUSE_TIMERS 0
#define configCHECK_FOR_STACK_OVERFLOW 2
#define configUSE_MALLOC_FAILED_HOOK 1
/* Plain integer literals: RVDS port also expands these in ARM assembly. */
#define configKERNEL_INTERRUPT_PRIORITY 240
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 80
#define INCLUDE_vTaskDelay 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
#define xPortSysTickHandler SysTick_Handler
void App_AssertFailed(void);
#define configASSERT(x) do { if (!(x)) App_AssertFailed(); } while (0)
#endif
