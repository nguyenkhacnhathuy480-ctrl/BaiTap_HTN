TARGET = main

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CFLAGS = -mcpu=cortex-m3 \
         -mthumb \
         -O2 \
         -Wall \
         -g

LDFLAGS = -T linker.ld \
          -nostdlib \
          -Wl,--gc-sections

SOURCES = main.c startup.s

OBJECTS = main.o startup.o

all: $(TARGET).bin
	$(SIZE) $(TARGET).elf

$(TARGET).elf: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $(TARGET).elf

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $(TARGET).elf $(TARGET).bin

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

startup.o: startup.s
	$(CC) $(CFLAGS) -c startup.s -o startup.o

clean:
	rm -f *.o
	rm -f *.elf
	rm -f *.bin

flash: $(TARGET).bin
	st-flash write $(TARGET).bin 0x08000000

.PHONY: all clean flash