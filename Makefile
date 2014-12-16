all : program.hex burn program.lst
CC = avr-gcc

OBJS = test.o usb.o SPIPrinting.o
SRCS = test.c usb.c SPIPrinting.c

PROCESSOR=atmega8u2
PROGRAMCODE=m8u2
CFLAGS = -Wall -Os -mmcu=$(PROCESSOR) -DF_CPU=16000000UL -I. -Iusbdrv
ASFLAGS = $(CFLAGS) -x assembler-with-cpp

program.elf : $(OBJS)
	avr-gcc -I -mmcu=$(PROCESSOR) $(CFLAGS) -Wl,-Map,program.map -o $@ $^ -L /usr/lib64/binutils/avr/2.19.1

program.hex : program.elf
	avr-objcopy -j .text -j .data -O ihex program.elf program.hex 

program.lst : $(SRCS)
	avr-gcc -c -g -Wa,-a,-ad $(CFLAGS) $^ > $@

burn : program.hex
	avrdude -c usbtiny -p $(PROGRAMCODE) -U flash:w:program.hex -F


readfuses :
	avrdude -c usbtiny -p $(PROGRAMCODE) -U hfuse:r:high.txt:b -U lfuse:r:low.txt:b

burnfuses :
	avrdude -c usbtiny -p $(PROGRAMCODE) -U lfuse:w:0xEE:m -U hfuse:w:0xD9:m -U efuse:w:0xCC:m
#Setup clock / Disable hardware booter - we want the SPI Programmer only!

clean : 
	rm -f *~ high.txt low.txt program.hex program.map program.elf $(OBJS) *.o program.lst

