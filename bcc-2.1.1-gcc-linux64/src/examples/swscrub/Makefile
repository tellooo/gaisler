PREFIX=sparc-gaisler-elf-
CC = $(PREFIX)gcc
LD = $(PREFIX)gcc
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size

CFLAGS   =
CFLAGS  += -Os
CFALGS  += -std=c99 -Wall -Wextra -pedantic
CFLAGS  += -Imscrub/include
CFLAGS  += -mcpu=leon3

.PHONY: all

TESTS = swscrub.elf

all: swscrub.elf swscrub.dis size

.phony: size
size: $(TESTS)
	$(SIZE) $(TESTS)

swscrub.elf: swscrub.c mscrub/mscrub.c mscrub/mscrub_access.s
	$(CC) $(CFLAGS) -o $@ $^ -Wl,-Map=$@.map

swscrub.dis: swscrub.elf
	$(OBJDUMP) -d -r $< > $@

clean:
	rm -f *.map *.dis $(TESTS)

