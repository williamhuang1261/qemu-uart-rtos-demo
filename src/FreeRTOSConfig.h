#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*
 * Minimal FreeRTOSConfig.h for the ARM_CM3 GCC port running on QEMU's
 * mps2-an385 machine. Values follow the conventions used across FreeRTOS's
 * official Cortex-M3 demos; only what this project actually uses is turned
 * on.
 */

#define configUSE_PREEMPTION                   1
#define configUSE_IDLE_HOOK                    0
#define configUSE_TICK_HOOK                    0
#define configCPU_CLOCK_HZ                     ( 25000000UL )
#define configTICK_RATE_HZ                     ( 1000 )
#define configMAX_PRIORITIES                   ( 5 )
#define configMINIMAL_STACK_SIZE                ( 128 )
#define configTOTAL_HEAP_SIZE                  ( ( size_t ) ( 16 * 1024 ) )
#define configMAX_TASK_NAME_LEN                ( 16 )
#define configUSE_16_BIT_TICKS                 0
#define configIDLE_SHOULD_YIELD                1
#define configUSE_MUTEXES                      1
#define configUSE_RECURSIVE_MUTEXES            0
#define configUSE_COUNTING_SEMAPHORES          0
#define configQUEUE_REGISTRY_SIZE              4
#define configUSE_QUEUE_SETS                   0
#define configUSE_TIME_SLICING                 1
#define configUSE_TIMERS                       0
#define configCHECK_FOR_STACK_OVERFLOW         2
#define configUSE_MALLOC_FAILED_HOOK           1
#define configUSE_TRACE_FACILITY               0
#define configGENERATE_RUN_TIME_STATS          0

/* Co-routines are unused by this project. */
#define configUSE_CO_ROUTINES                  0
#define configMAX_CO_ROUTINE_PRIORITIES        1

/* Task API subset actually used. */
#define INCLUDE_vTaskPrioritySet               0
#define INCLUDE_uxTaskPriorityGet              0
#define INCLUDE_vTaskDelete                    0
#define INCLUDE_vTaskSuspend                   1
#define INCLUDE_vTaskDelayUntil                1
#define INCLUDE_vTaskDelay                     1
#define INCLUDE_xTaskGetSchedulerState         1

/* Cortex-M3 interrupt priority setup: the standard values used across
 * FreeRTOS's own Cortex-M demos, valid for any implemented NVIC priority
 * bit width because only the top bits are ever significant. */
#define configPRIO_BITS                        4
#define configKERNEL_INTERRUPT_PRIORITY        255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY   0xB0 /* priority 5, 4 significant bits, LSBs clear */
#define configMAX_API_CALL_INTERRUPT_PRIORITY  configMAX_SYSCALL_INTERRUPT_PRIORITY

#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for( ;; ); }

/* CMSDK-based QEMU boards do not provide a CMSIS device header defining the
 * NVIC set-priority helpers the port expects by default names; the ARM_CM3
 * port only needs the handler names below to line up with the vector table
 * in startup_mps2an385.c. */
#define vPortSVCHandler                        SVC_Handler
#define xPortPendSVHandler                     PendSV_Handler
#define xPortSysTickHandler                    SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
