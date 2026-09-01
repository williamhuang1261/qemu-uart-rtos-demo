CC      := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE    := arm-none-eabi-size

CPU_FLAGS := -mcpu=cortex-m3 -mthumb
CFLAGS    := $(CPU_FLAGS) -O2 -g -ffreestanding -fno-builtin -Wall -Wextra \
             -ffunction-sections -fdata-sections -Isrc
LDFLAGS   := $(CPU_FLAGS) -T linker/mps2an385.ld -nostartfiles -nostdlib \
             -Wl,--gc-sections -Wl,-Map=build/firmware.map

BUILD := build
SRCS  := startup/startup_mps2an385.c src/uart.c src/main.c
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
