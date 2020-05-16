# Name of binary to create.
PROJ = ex.elf

HOST=sparc-gaisler-elf
CC=$(HOST)-gcc
AS=$(HOST)-as
OBJDUMP=$(HOST)-objdump
SIZE=$(HOST)-size

INCFLAGS  =
INCFLAGS += -I.
INCFLAGS += -Iinclude
INCFLAGS += -Ifl/include
CFLAGS  =
CFLAGS += -std=c99 -Wall -Wextra -pedantic
CFLAGS += -O2
CFLAGS += -qbsp=gr712rc
CFLAGS += -mcpu=leon3
CFLAGS += -mfix-gr712rc
CFLAGS += $(INCFLAGS)

VPATH = fl
# Default target
all: $(PROJ) buildsafe

MYOBJ  =
MYOBJ += main.o
MYOBJ += fl.o
MYOBJ += fl_i8.o
MYOBJ += fl_amd8.o
MYOBJ += fl_amd32.o
fl.o: fl_priv.h
fl_i8.o: fl_priv.h
fl_amd8.o: fl_priv.h
fl_amd32.o: fl_priv.h
$(MYOBJ): fl/include/fl.h
$(PROJ): $(MYOBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

.PHONY: buildsafe
buildsafe:
	$(MAKE) -C safe/

.PHONY: cleansafe
cleansafe:
	$(MAKE) -C safe/ clean

.PHONY: clean
clean: cleansafe
	$(RM) $(PROJ)
	$(RM) *.o

