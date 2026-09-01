#include <stdint.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "protocol.h"
#include "tasks_app.h"
#include "uart.h"

#define APP_QUEUE_LENGTH 8

typedef struct {
    uint8_t buf[PROTOCOL_MAX_FRAME];
    uint8_t len;
} queued_frame_t;

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

static char *append_str(char *dst, const char *s)
{
    while (*s) {
        *dst++ = *s++;
    }
    return dst;
}

/* payload layout: byte 0 = producer id, bytes 1..4 = seq, little-endian. */
static uint8_t encode_seq_payload(uint8_t *payload, char id, uint32_t seq)
{
    payload[0] = (uint8_t)id;
    payload[1] = (uint8_t)(seq & 0xFFu);
    payload[2] = (uint8_t)((seq >> 8) & 0xFFu);
    payload[3] = (uint8_t)((seq >> 16) & 0xFFu);
    payload[4] = (uint8_t)((seq >> 24) & 0xFFu);
    return 5;
}

static void producer_task(void *pvParameters)
{
    char id = *(const char *)pvParameters;
    TickType_t period = (id == 'A') ? pdMS_TO_TICKS(300) : pdMS_TO_TICKS(700);
    uint32_t seq = 0;

    for (;;) {
        uint8_t payload[PROTOCOL_MAX_PAYLOAD];
        uint8_t payload_len = encode_seq_payload(payload, id, seq);

        queued_frame_t qf;
        qf.len = (uint8_t)protocol_encode(qf.buf, sizeof(qf.buf), payload, payload_len);

        /* Every 5th frame from producer B is deliberately corrupted after
         * encoding (a single bit flipped in the checksum byte) so the
         * consumer's checksum rejection path is genuinely exercised, not
         * just written and never triggered. */
        if (id == 'B' && seq % 5 == 4) {
            qf.buf[qf.len - 1] ^= 0x01u;
        }

        /* A full queue means the consumer has fallen behind; block rather
         * than silently drop so backpressure is visible instead of hidden. */
        xQueueSend(xAppQueue, &qf, portMAX_DELAY);
        seq++;
        vTaskDelay(period);
    }
}

static void consumer_task(void *pvParameters)
{
    (void)pvParameters;
    queued_frame_t qf;
    uint8_t payload[PROTOCOL_MAX_PAYLOAD];
    uint8_t payload_len;
    char line[64];

    for (;;) {
        if (xQueueReceive(xAppQueue, &qf, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        protocol_status_t status = protocol_decode(qf.buf, qf.len, payload, &payload_len);
        char *p = line;

        if (status == PROTOCOL_OK) {
            char id = (char)payload[0];
            uint32_t seq = (uint32_t)payload[1] | ((uint32_t)payload[2] << 8) |
                           ((uint32_t)payload[3] << 16) | ((uint32_t)payload[4] << 24);
            p = append_str(p, "[consumer] frame OK from ");
            *p++ = id;
            p = append_str(p, " seq=");
            p = append_uint(p, seq);
        } else {
            p = append_str(p, "[consumer] frame REJECTED (");
            p = append_str(p, status == PROTOCOL_ERR_BAD_CHECKSUM ? "bad checksum" : "malformed");
            p = append_str(p, ")");
        }
        *p++ = '\n';
        *p = '\0';
        uart_puts(line);
    }
}

static char producer_a_id = 'A';
static char producer_b_id = 'B';

void app_tasks_create(void)
{
    xAppQueue = xQueueCreate(APP_QUEUE_LENGTH, sizeof(queued_frame_t));
    configASSERT(xAppQueue != NULL);

    xTaskCreate(producer_task, "producerA", configMINIMAL_STACK_SIZE, &producer_a_id,
                tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(producer_task, "producerB", configMINIMAL_STACK_SIZE, &producer_b_id,
                tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(consumer_task, "consumer", configMINIMAL_STACK_SIZE, NULL,
                tskIDLE_PRIORITY + 2, NULL);
}
