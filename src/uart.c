/*
 * Driver for the ARM CMSDK APB UART, register-compatible with the UART0
 * QEMU's mps2-an385 machine model exposes at 0x40004000. This is the same
 * register layout described in the ARM CMSDK Technical Reference Manual,
 * so this driver is unmodified from what would run on the real AN385 FPGA
 * image.
 */
#include <stdint.h>
#include "uart.h"

#define UART0_BASE 0x40004000u

struct cmsdk_uart {
    volatile uint32_t DATA;
    volatile uint32_t STATE;
    volatile uint32_t CTRL;
    volatile uint32_t INTSTATUS;
    volatile uint32_t BAUDDIV;
};

#define UART0 ((struct cmsdk_uart *)UART0_BASE)

#define UART_STATE_TX_FULL   (1u << 0)
#define UART_CTRL_TX_ENABLE  (1u << 0)
#define UART_CTRL_RX_ENABLE  (1u << 1)

void uart_init(void)
{
    /* Minimum valid divider per the CMSDK UART TRM; QEMU's model does not
     * simulate real baud timing, but a legal value is set so this driver
     * is unchanged if it ever runs against real silicon. */
    UART0->BAUDDIV = 16;
    UART0->CTRL = UART_CTRL_TX_ENABLE | UART_CTRL_RX_ENABLE;
}

void uart_putc(char c)
{
    while (UART0->STATE & UART_STATE_TX_FULL) {
        /* wait for space in the transmit buffer */
    }
    UART0->DATA = (uint32_t)(unsigned char)c;
}

void uart_puts(const char *s)
{
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
}
