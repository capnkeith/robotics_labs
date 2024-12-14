# Sample Makefile for ATmega328P using avr-gcc

# Define the target and MCU
TARGET = main
MCU = avr64da64
F_CPU = 24000000
CDEFS = -DF_CPU=$(F_CPU)UL

# Define the compiler and avrdude paths
CC = avr-gcc
AVRDUDE = avrdude

# Define the programmer and port
PROGRAMMER = serialupdi
PORT = /dev/tty.usbserial-1430

# Define the compiler flags
CFLAGS = -Wall -Os -std=gnu99 -mmcu=$(MCU) $(CDEFS)
CPPFLAGS = -Wall -Os -std=c++11 -mmcu=$(MCU) $(CDEFS)

# Define the linker flags
LDFLAGS = -mmcu=$(MCU)

# Define the source files
CSOURCES = light_ws2812.c
CPPSOURCES = led.cpp

# Define the object files
OBJECTS = $(CSOURCES:.c=.o) $(CPPSOURCES:.cpp=.o)

# Default target
all: $(TARGET).hex

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile C++ files
%.o: %.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $<

# Link object files
$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^ 

# Create hex file
$(TARGET).hex: $(TARGET).elf
	avr-objcopy -O ihex -R .eeprom $(TARGET).elf $@

# Upload the hex file to the MCU
program: $(TARGET).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -U flash:w:$(TARGET).hex:i

# Clean the project
clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).hex
