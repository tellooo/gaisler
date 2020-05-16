PREFIX=sparc-gaisler-elf-
CC = $(PREFIX)gcc
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size
CCOPT = -O2
BSP = leon3
# BSP = gr712rc
CFLAGS = -qbsp=$(BSP) -mcpu=leon3 -qnano -std=c99 -Wall -Wextra -pedantic $(CCOPT) $(LDFLAGS)
#CFLAGS += -qsvt
LDFLAGS = -T linkcmds-rom

PROJ=ex
SOURCES  =

all: $(PROJ).elf $(PROJ).dis

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $^ -o $@

$(PROJ).elf: main.o init40.o
	$(CC) $(CFLAGS) -Wl,-Map=$(PROJ).map $^ -o $@

$(PROJ).dis: $(PROJ).elf
	$(OBJDUMP) -d $^ > $@

size: $(PROJ).elf
	$(SIZE) $^

clean:
	rm -f *.elf *.map *.dis
	rm -f *.o

