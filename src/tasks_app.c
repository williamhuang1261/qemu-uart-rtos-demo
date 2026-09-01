#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "tasks_app.h"
#include "uart.h"

#define APP_QUEUE_LENGTH 8

typedef struct {
    char producer_id;
    uint32_t seq;
} app_message_t;

static QueueHandle_t xAppQueue;

/* No libc <stdio.h> is linked (freestanding, -nostdlib), so decimal
 * formatting is a few lines of hand-rolled code rather than snprintf. */
static char *append_uint(char *dst, uint32_t value)
{
    char digits[10];
    int n = 0;

    if (value == 0) {
        *dst++ = '0';
        return dst;
    }
    while (value > 0) {
        digits[n++] = (char)('0' + (value % 10));
        value /= 10;
    }
    while (n > 0) {
        *dst++ = digits[--n];
    }
    return dst;
}

static void producer_task(void *pvParameters)
{
    char id = *(const char *)pvParameters;
    TickType_t period = (id == 'A') ? pdMS_TO_TICKS(300) : pdMS_TO_TICKS(700);
    uint32_t seq = 0;

    for (;;) {
        app_message_t msg = { .producer_id = id, .seq = seq };
        /* A full queue means the consumer has fallen behind; block rather
         * than silently drop so backpressure is visible instead of hidden. */
        xQueueSend(xAppQueue, &msg, portMAX_DELAY);
        seq++;
        vTaskDelay(period);
    }
}

static void consumer_task(void *pvParameters)
{
    (void)pvParameters;
    app_message_t msg;
    char line[48];

    for (;;) {
        if (xQueueReceive(xAppQueue, &msg, portMAX_DELAY) == pdTRUE) {
            char *p = line;
            const char *prefix = "[consumer] from ";
            while (*prefix) {
                *p++ = *prefix++;
            }
            *p++ = msg.producer_id;
            const char *mid = " seq=";
            while (*mid) {
                *p++ = *mid++;
            }
            p = append_uint(p, msg.seq);
            *p++ = '\n';
            *p = '\0';
            uart_puts(line);
        }
    }
}

static char producer_a_id = 'A';
static char producer_b_id = 'B';

void app_tasks_create(void)
{
    xAppQueue = xQueueCreate(APP_QUEUE_LENGTH, sizeof(app_message_t));
    configASSERT(xAppQueue != NULL);

    xTaskCreate(producer_task, "producerA", configMINIMAL_STACK_SIZE, &producer_a_id,
                tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(producer_task, "producerB", configMINIMAL_STACK_SIZE, &producer_b_id,
                tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(consumer_task, "consumer", configMINIMAL_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 2, NULL);
}
