TARGET := bai4
BUILD := build
CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
SIZE := arm-none-eabi-size

CPUFLAGS := -mcpu=cortex-m3 -mthumb
CFLAGS := $(CPUFLAGS) -std=c11 -O2 -g3 -Wall -Wextra -Werror \
          -ffreestanding -fdata-sections -ffunction-sections -MMD -MP
ASFLAGS := $(CPUFLAGS) -x assembler-with-cpp
LDFLAGS := $(CPUFLAGS) -T linker.ld -nostdlib \
           -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(TARGET).map \
           -Wl,--print-memory-usage

OBJECTS := $(BUILD)/main.o $(BUILD)/startup.o

.PHONY: all clean flash

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin

$(BUILD):
	mkdir -p $@

$(BUILD)/main.o: main.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/startup.o: startup.s | $(BUILD)
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJECTS) linker.ld
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: all
	st-flash --flash=32k write $(BUILD)/$(TARGET).bin 0x08000000

clean:
	rm -rf $(BUILD)

-include $(BUILD)/main.d
