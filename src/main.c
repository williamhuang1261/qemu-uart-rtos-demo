#include <stdint.h>
#include "uart.h"

static void busy_delay(volatile uint32_t count)
{
    while (count--) {
        __asm__ volatile("nop");
    }
}

int main(void)
{
    uart_init();
    uart_puts("Hello from bare-metal Cortex-M3 on QEMU (mps2-an385)!\n");

    unsigned tick = 0;
    for (;;) {
        uart_puts("heartbeat\n");
        (void)tick++;
        busy_delay(2000000);
    }
}
