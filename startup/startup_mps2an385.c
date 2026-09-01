/*
 * Minimal Cortex-M3 vector table and reset handler for QEMU's mps2-an385
 * machine. No vendor CMSIS package is used: the vector table and startup
 * sequence are small enough to write and explain directly.
 */
#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

void Reset_Handler(void);
void Default_Handler(void);
int main(void);

void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)     __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void)    __attribute__((weak, alias("Default_Handler")));

/* Cortex-M3 exception table followed by the CMSDK UART/timer/GPIO IRQs.
 * Only the entries this project actually uses are named; the rest point at
 * the weak Default_Handler so an unexpected interrupt is visible (it spins)
 * rather than silently corrupting execution. */
__attribute__((section(".isr_vector")))
void (* const g_pfnVectors[])(void) = {
    (void (*)(void))(&_estack),   /* initial stack pointer */
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0, 0, 0, 0,                   /* reserved */
    SVC_Handler,
    DebugMon_Handler,
    0,                            /* reserved */
    PendSV_Handler,
    SysTick_Handler,
};

void Default_Handler(void)
{
    while (1) {
        /* Unexpected trap: spin so it is obvious under a debugger instead
         * of silently returning into garbage state. */
    }
}

void Reset_Handler(void)
{
    uint32_t *src, *dst;

    /* Copy .data out of FLASH (its load address, _etext) into RAM. */
    src = &_etext;
    dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* Zero .bss. */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    main();

    while (1) {
        /* main() should never return on bare metal. */
    }
}
