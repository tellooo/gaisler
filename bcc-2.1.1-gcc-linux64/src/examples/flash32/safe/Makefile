# Name of binary to create.
PROJ = safe.elf

HOST=sparc-gaisler-elf
CC=$(HOST)-gcc
AS=$(HOST)-as
OBJDUMP=$(HOST)-objdump
SIZE=$(HOST)-size

INCFLAGS = -I../include
CFLAGS  =
CFLAGS += -std=c99 -Wall -Wextra -pedantic
CFLAGS += -Os
CFLAGS += -qbsp=gr712rc
CFLAGS += -mcpu=leon3
CFLAGS += -mfix-gr712rc
CFLAGS += -msoft-float
CFLAGS += -mflat
CFLAGS += $(INCFLAGS)
LDFLAGS  =
LDFLAGS += -T linkcmds-ahbram
LDFLAGS += -nostartfiles

# Default target
all: $(PROJ)
all: $(PROJ).dis
all: size


MYOBJ  =
MYOBJ += safe_trap_table.o
MYOBJ += safe_data_access_exception.o
MYOBJ += safe_stubs.o
MYOBJ += safe_ops.o
MYOBJ += safe_soft_trap.o

%.dis: %.o
	$(OBJDUMP) -d -r $< > $@

%.o: %.S
	$(CC) $(CFLAGS) -c $^ -o $@

$(PROJ): $(MYOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(PROJ).dis: $(PROJ)
	$(OBJDUMP) -d -r $< > $@

.PHONY: size
size: $(PROJ)
	$(SIZE) $<

.PHONY: clean
clean:
	$(RM) $(PROJ)
	$(RM) *.o
	$(RM) *.dis

