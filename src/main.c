#include "FreeRTOS.h"
#include "task.h"

#include "tasks_app.h"
#include "uart.h"

int main(void)
{
    uart_init();
    uart_puts("Hello from FreeRTOS on QEMU (mps2-an385)!\n");

    app_tasks_create();

    vTaskStartScheduler();

    /* vTaskStartScheduler() only returns if there was not enough heap to
     * create the idle task, which would be a configuration bug. */
    uart_puts("FATAL: scheduler returned\n");
    for (;;) {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    uart_puts("FATAL: stack overflow\n");
    for (;;) {
    }
}

void vApplicationMallocFailedHook(void)
{
    uart_puts("FATAL: malloc failed\n");
    for (;;) {
    }
}
