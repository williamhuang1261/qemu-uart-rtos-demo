#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#include "uart.h"

static void heartbeat_task(void *pvParameters)
{
    (void)pvParameters;
    uint32_t beat = 0;

    for (;;) {
        uart_puts("heartbeat tick\n");
        (void)beat++;
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    uart_init();
    uart_puts("Hello from FreeRTOS on QEMU (mps2-an385)!\n");

    xTaskCreate(heartbeat_task, "heartbeat", configMINIMAL_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 1, NULL);

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
