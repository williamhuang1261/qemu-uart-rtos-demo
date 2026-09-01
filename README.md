# qemu-uart-rtos-demo

RTOS firmware for an emulated ARM Cortex-M3 board, built and debugged with
an entirely open-source toolchain: no vendor IDE, no physical hardware, no
paid tools.

Three FreeRTOS tasks (two producers, one consumer) coordinate over a shared
queue using a hand-written UART-style framed protocol (start byte, length,
payload, checksum). The firmware boots on QEMU's `mps2-an385` machine model
(ARM MPS2 development board + AN385 FPGA image) and is debugged live over
QEMU's GDB remote-serial protocol.

## Why this exists

It is a small, honest demonstration of the embedded-systems fundamentals a
firmware role actually exercises day to day: writing C against a real
memory-mapped peripheral, standing up an RTOS from source (not a vendor
package), a protocol with real error handling, and using a debugger against
a running target instead of `printf`-only debugging.

## Stack

| Piece | What / why |
| --- | --- |
| **arm-none-eabi-gcc** | Cross-compiler targeting `cortex-m3`/Thumb, no OS assumptions (`-ffreestanding -nostdlib`). |
| **FreeRTOS-Kernel** (vendored, `third_party/FreeRTOS-Kernel`, git submodule) | The ARM_CM3 GCC port, `heap_4` allocator, `tasks`/`queue`/`list`. Real upstream source, not a hand-rolled scheduler. |
| **QEMU (`mps2-an385` machine)** | Free, cycle-approximate emulation of a real Cortex-M3 dev board; the CMSDK UART QEMU implements is register-for-register the same as the real AN385 FPGA image, so `src/uart.c` would run unmodified on real hardware. |
| **GDB** | Standard embedded remote-debug workflow: `qemu-system-arm -s -S` starts a gdbserver on `:1234`; GDB attaches, sets breakpoints, and inspects state on the live target. |

No vendor CMSIS package is used. The vector table (`startup/startup_mps2an385.c`)
and linker script (`linker/mps2an385.ld`) are written directly against the
Cortex-M3 architecture and the AN385 memory map, which is small enough to
read end to end.

## What it does

- **`startup/startup_mps2an385.c`** — the Cortex-M3 vector table and reset
  handler: copies `.data` out of flash, zeroes `.bss`, calls `main()`.
- **`src/uart.c`** — a driver for the ARM CMSDK APB UART (register layout
  per the CMSDK Technical Reference Manual) at `0x40004000`.
- **`src/tasks_app.c`** — two producer tasks (300ms and 700ms periods) each
  encode a frame and push it onto a shared `xQueueCreate`d queue; one
  consumer task drains it, decodes, and prints the result over UART.
- **`src/protocol.c`** / **`src/protocol.h`** — the framed protocol:
  `START(0x7E) | LENGTH | PAYLOAD[LENGTH] | CHECKSUM`. Producer B corrupts
  the checksum byte on every 5th frame on purpose, so the consumer's
  rejection path is genuinely exercised rather than just written.

## Building and running

Requires `arm-none-eabi-gcc` **with newlib** (Homebrew's `arm-none-eabi-gcc`
formula ships the compiler only — see [Engineering notes](#engineering-notes))
and `qemu-system-arm` (`brew install qemu`).

```sh
git clone --recurse-submodules https://github.com/williamhuang1261/qemu-uart-rtos-demo.git
cd qemu-uart-rtos-demo
make                       # builds build/firmware.elf
make run                   # boots it under QEMU; Ctrl-A X to quit
```

If your `arm-none-eabi-gcc` has no newlib (Homebrew's formula does not),
point `make` at a full toolchain instead:

```sh
make TOOLCHAIN_BIN=/path/to/arm-gnu-toolchain-*/bin/
```

### Sample output

```
Hello from FreeRTOS on QEMU (mps2-an385)!
[consumer] frame OK from A seq=0
[consumer] frame OK from B seq=0
[consumer] frame OK from A seq=1
[consumer] frame OK from A seq=2
[consumer] frame OK from B seq=1
[consumer] frame OK from A seq=3
[consumer] frame OK from A seq=4
[consumer] frame OK from B seq=2
[consumer] frame OK from A seq=5
[consumer] frame OK from A seq=6
[consumer] frame OK from B seq=3
[consumer] frame OK from A seq=7
[consumer] frame OK from A seq=8
[consumer] frame OK from A seq=9
[consumer] frame REJECTED (bad checksum)
[consumer] frame OK from A seq=10
```

Producer A (300ms) and producer B (700ms) interleave through the shared
queue exactly as their periods predict, and B's 5th frame (`seq=4`) is the
one that gets rejected — proof the checksum path actually runs both ways.

### Automated test

```sh
bash scripts/run_qemu_test.sh
```

Boots QEMU headless for 4 seconds and checks the captured serial output for
the boot banner, accepted frames from both producers, and the rejected
checksum line. Prints `PASS: ...` and exits 0 on success.

### Debugging with GDB

```sh
make debug                 # starts QEMU, halted, gdbserver on :1234
# in another terminal:
arm-none-eabi-gdb -q build/firmware.elf -x scripts/gdb_session.gdb
```

A real, unedited transcript from this exact workflow — breakpoint on
`protocol_decode`, a backtrace, and `finish` showing the live return
value — is saved at [`docs/gdb_session.txt`](docs/gdb_session.txt).

## Engineering notes

**Homebrew's `arm-none-eabi-gcc` ships no libc.** The formula installs the
compiler and binutils only — no newlib headers or libraries — so linking
with `-nostdlib` (correct for this project; a hosted libc would need syscall
stubs like `_sbrk`/`_write` this firmware has no OS underneath to satisfy)
still fails on `<stdlib.h>` itself, which the Homebrew build doesn't ship at
all. Rather than pull in a whole newlib syscall layer for a firmware that
never needs `malloc`/file I/O from libc, the project vendors ARM's full GNU
toolchain release locally for its headers (`Makefile`'s `TOOLCHAIN_BIN`
override) and hand-writes the three libc functions FreeRTOS actually calls
(`memset`, `memcpy`, `strlen` — see `src/libc_stubs.c`, a dozen lines
total). This keeps the binary genuinely freestanding and makes exactly what
touches libc visible in one small file, instead of hiding it behind a
partially-stubbed newlib.

**The UART driver polls; it does not use interrupts.** `uart_putc()` spins
on the CMSDK UART's `STATE` register until the transmit buffer has room.
For three cooperating tasks printing short lines, the wait is negligible
and an interrupt-driven TX path would add a ring buffer and an ISR for no
observable benefit here. A receive path (not implemented — this firmware
only transmits) is the case that would actually need interrupts, to avoid
losing bytes while a task is busy elsewhere.

**One real bug worth naming:** early in development,
`configMAX_SYSCALL_INTERRUPT_PRIORITY` was written as decimal `191` when
the intended value was `0xB0` (176) — FreeRTOS's own `configASSERT` caught
the mismatched low bits and the CPU spun silently in the assert-failure
loop. Found by attaching GDB to QEMU's gdbserver and reading the live
program counter, not by guessing.

## What this deliberately does not cover

- No real hardware — QEMU's `mps2-an385` model is accurate but not a
  substitute for verifying against a physical board and a logic analyzer.
- No RX path on the UART (see Engineering notes above).
- The checksum is a simple XOR, chosen for readability over robustness; a
  real link would likely use a CRC.
