/*
 * Linker script for QEMU's mps2-an385 machine (Cortex-M3, ARM MPS2 + AN385
 * FPGA image). Memory map matches the AN385 application note: 4 MiB of
 * boot/code memory at 0x00000000 and 4 MiB of data SRAM at 0x20000000.
 */

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x00000000, LENGTH = 4M
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 4M
}

ENTRY(Reset_Handler)

SECTIONS
{
    .isr_vector :
    {
        . = ALIGN(4);
        KEEP(*(.isr_vector))
        . = ALIGN(4);
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
    } > FLASH

    _etext = .;

    .data : AT (_etext)
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    . = ALIGN(8);
    _sheap = .;
    . = . + 0x2000; /* 8 KiB heap, used by FreeRTOS heap_4 in later steps */
    _eheap = .;

    . = ALIGN(8);
    _estack = ORIGIN(RAM) + LENGTH(RAM);
}
