TARGET := bai-03-adc-uart
DRIVERS_DIR := ../Drivers
CMSIS_DEVICE_DIR := $(DRIVERS_DIR)/CMSIS/Device/ST/STM32F1xx

BUILD_DIR := build
PREFIX := arm-none-eabi-
CC := $(PREFIX)gcc
AS := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
SIZE := $(PREFIX)size

CPU_FLAGS := -mcpu=cortex-m3 -mthumb
DEFINES := -DSTM32F103x6
INCLUDES := \
	-I$(DRIVERS_DIR)/CMSIS/Include \
	-I$(CMSIS_DEVICE_DIR)/Include

CFLAGS := $(CPU_FLAGS) $(DEFINES) $(INCLUDES) \
	-std=c11 -O2 -g3 -Wall -Wextra -Werror \
	-ffunction-sections -fdata-sections -MMD -MP
ASFLAGS := $(CPU_FLAGS) $(DEFINES) $(INCLUDES) -x assembler-with-cpp
LDSCRIPT := $(CMSIS_DEVICE_DIR)/Source/Templates/gcc/STM32F103X6_FLASH.ld
LDFLAGS := $(CPU_FLAGS) -T$(LDSCRIPT) --specs=nano.specs --specs=nosys.specs \
	-Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
	-Wl,--print-memory-usage

C_SOURCES := Core/Src/main.c \
	$(CMSIS_DEVICE_DIR)/Source/Templates/system_stm32f1xx.c
ASM_SOURCES := $(CMSIS_DEVICE_DIR)/Source/Templates/gcc/startup_stm32f103x6.s
OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o))) \
	$(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

.PHONY: all clean flash uart

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(LDSCRIPT)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: all
	st-flash --flash=64k write $(BUILD_DIR)/$(TARGET).bin 0x08000000

uart:
	picocom -b 115200 /dev/ttyUSB0

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d)
