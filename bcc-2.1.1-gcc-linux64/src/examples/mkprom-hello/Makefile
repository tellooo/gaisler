PREFIX0=sparc-gaisler-elf
PREFIX=$(PREFIX0)-
CC = $(PREFIX)gcc
OBJDUMP = $(PREFIX)objdump
MKPROM = mkprom2
MKPROMOPT ?= -freq 50
MKPROMFLAGS = -v -ccprefix $(PREFIX0) $(MKPROMOPT)

CCOPT = -Os -qnano
BSP = leon3
CFLAGS = -qbsp=$(BSP) -std=c99 -Wall -Wextra -pedantic $(CCOPT)

PROJ=hello
SOURCES = main.c

all: $(PROJ).elf $(PROJ).dis $(PROJ).prom

$(PROJ).elf: $(SOURCES)
	$(CC) $(CFLAGS) -Wl,-Map=$(PROJ).map $^ -o $@

$(PROJ).dis: $(PROJ).elf
	$(OBJDUMP) -d $^ > $@

$(PROJ).prom: $(PROJ).elf
	$(MKPROM) $(MKPROMFLAGS) -o $@ $^

clean:
	rm -f $(PROJ).elf $(PROJ).map $(PROJ).dis $(PROJ).prom xdump.s

