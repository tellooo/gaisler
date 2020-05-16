PREFIX=sparc-gaisler-elf-
CC = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

CFLAGS   =
CFLAGS  += -O3
CFALGS  += -std=c99 -Wall -Wextra -pedantic
# CFLAGS  += -qbsp=gr740
CFLAGS  += -qbsp=gr716
CFLAGS  += -mcpu=leon3

ASFLAGS = $(CFLAGS)

.PHONY: all

all: m.elf m.dis size

.phony: size
size: m.elf
	$(SIZE) $<

m.elf: main.o meas.o
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Map=$@.map
meas.o: meas.S
	$(CC) $(CFLAGS) -c -o $@ $<
main.o: main.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.dis: %.elf
	$(OBJDUMP) -d -r $< > $@

clean:
	rm -f *.map *.dis *.o *.elf

