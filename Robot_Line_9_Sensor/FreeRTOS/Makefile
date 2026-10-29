TARGET := robot-line-9-sensor-freertos
BUILD_DIR := build

CMSIS_DIR := ../Drivers/CMSIS
CMSIS_DEVICE_DIR := $(CMSIS_DIR)/Device/ST/STM32F1xx
FREERTOS_DIR := ../Middleware/FreeRTOS

PREFIX := arm-none-eabi-
CC := $(PREFIX)gcc
AS := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
SIZE := $(PREFIX)size

CPU_FLAGS := -mcpu=cortex-m3 -mthumb
DEFINES := -DSTM32F103xB
INCLUDES := \
	-I../Core/Inc \
	-ICore/Inc \
	-I$(CMSIS_DIR)/Include \
	-I$(CMSIS_DEVICE_DIR)/Include \
	-I$(FREERTOS_DIR)/include \
	-I$(FREERTOS_DIR)/portable/GCC/ARM_CM3

CFLAGS := $(CPU_FLAGS) $(DEFINES) $(INCLUDES) \
	-std=c11 -O2 -g3 -Wall -Wextra -Werror \
	-ffunction-sections -fdata-sections -MMD -MP
ASFLAGS := $(CPU_FLAGS) $(DEFINES) $(INCLUDES) -x assembler-with-cpp
LDFLAGS := $(CPU_FLAGS) -T../Startup/STM32F103C8_FLASH.ld \
	--specs=nano.specs --specs=nosys.specs \
	-Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map \
	-Wl,--print-memory-usage

C_SOURCES := \
	Core/Src/main_freertos.c \
	Core/Src/platform_freertos.c \
	../Core/Src/line_sensor.c \
	../Core/Src/motor_tb6612.c \
	../Core/Src/system_stm32f1xx.c \
	$(FREERTOS_DIR)/tasks.c \
	$(FREERTOS_DIR)/list.c \
	$(FREERTOS_DIR)/queue.c \
	$(FREERTOS_DIR)/portable/GCC/ARM_CM3/port.c \
	$(FREERTOS_DIR)/portable/MemMang/heap_4.c
ASM_SOURCES := ../Startup/startup_stm32f103xb.s
OBJECTS := $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o))) \
	$(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

.PHONY: all clean flash

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	$(AS) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) ../Startup/STM32F103C8_FLASH.ld
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: all
	st-flash --flash=64k write $(BUILD_DIR)/$(TARGET).bin 0x08000000

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJECTS:.o=.d)
