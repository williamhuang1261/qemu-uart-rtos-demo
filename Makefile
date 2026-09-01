# Homebrew's arm-none-eabi-gcc formula ships the compiler only (no newlib
# headers/libs). TOOLCHAIN_BIN can point at a full ARM GNU Toolchain
# distribution (https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
# instead; override on the command line or export it, e.g.:
#   make TOOLCHAIN_BIN=/path/to/arm-gnu-toolchain-*/bin/
TOOLCHAIN_BIN ?=

CC      := $(TOOLCHAIN_BIN)arm-none-eabi-gcc
OBJCOPY := $(TOOLCHAIN_BIN)arm-none-eabi-objcopy
SIZE    := $(TOOLCHAIN_BIN)arm-none-eabi-size

FREERTOS := third_party/FreeRTOS-Kernel
FREERTOS_PORT := $(FREERTOS)/portable/GCC/ARM_CM3

CPU_FLAGS := -mcpu=cortex-m3 -mthumb
CFLAGS    := $(CPU_FLAGS) -O2 -g -ffreestanding -fno-builtin -Wall -Wextra \
             -ffunction-sections -fdata-sections \
             -Isrc -I$(FREERTOS)/include -I$(FREERTOS_PORT)
LDFLAGS   := $(CPU_FLAGS) -T linker/mps2an385.ld -nostartfiles -nostdlib \
             -Wl,--gc-sections -Wl,-Map=build/firmware.map

BUILD := build
SRCS  := startup/startup_mps2an385.c \
         src/uart.c \
         src/libc_stubs.c \
         src/tasks_app.c \
         src/main.c \
         $(FREERTOS)/tasks.c \
         $(FREERTOS)/list.c \
         $(FREERTOS)/queue.c \
         $(FREERTOS)/portable/MemMang/heap_4.c \
         $(FREERTOS_PORT)/port.c
OBJS  := $(patsubst %.c,$(BUILD)/%.o,$(SRCS))

ELF := $(BUILD)/firmware.elf
BIN := $(BUILD)/firmware.bin

.PHONY: all clean run debug

all: $(ELF)

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJS) linker/mps2an385.ld
	$(CC) $(LDFLAGS) $(OBJS) -o $@
	$(OBJCOPY) -O binary $@ $(BIN)
	$(SIZE) $@

run: $(ELF)
	qemu-system-arm -M mps2-an385 -kernel $(ELF) -nographic -serial mon:stdio

debug: $(ELF)
	qemu-system-arm -M mps2-an385 -kernel $(ELF) -nographic -serial mon:stdio -s -S

clean:
	rm -rf $(BUILD)
