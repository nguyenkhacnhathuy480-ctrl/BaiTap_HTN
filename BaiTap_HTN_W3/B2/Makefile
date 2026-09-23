CC = arm-none-eabi-gcc

CFLAGS = -mcpu=cortex-m3 \
         -mthumb \
         -O2 \
         -Wall \
         -g

LDFLAGS = -T linker.ld \
          -nostdlib \
          -Wl,--gc-sections

TARGET = main

all: $(TARGET).bin

$(TARGET).elf: main.c startup.s linker.ld
	$(CC) $(CFLAGS) $(LDFLAGS) main.c startup.s -o $(TARGET).elf

$(TARGET).bin: $(TARGET).elf
	arm-none-eabi-objcopy -O binary $(TARGET).elf $(TARGET).bin

clean:
	rm -f *.elf *.bin *.o

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000
