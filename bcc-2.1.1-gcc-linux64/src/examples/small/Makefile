PREFIX=sparc-gaisler-elf-
CC = $(PREFIX)gcc
SIZE = $(PREFIX)size
OBJDUMP = $(PREFIX)objdump
CCOPT = -Os
LDFLAGS = -qsvt
BSP = gr716
CFLAGS = -qbsp=$(BSP) -mcpu=leon3 -qnano -std=c99 -Wall -Wextra -pedantic $(CCOPT) $(LDFLAGS)

PROJ=small
SOURCES  = 
SOURCES += main.c

all: $(PROJ).elf $(PROJ).dis
all: size
.PHONY: size
size: small.elf
	$(SIZE) $^

$(PROJ).elf: $(SOURCES)
	$(CC) $(CFLAGS) -Wl,-Map=$(PROJ).map $^ -o $@

$(PROJ).dis: $(PROJ).elf
	$(OBJDUMP) -d $^ > $@

clean:
	rm -f $(PROJ).elf $(PROJ).map $(PROJ).dis $(PROJ).tcl

$(PROJ).tcl: $(PROJ).elf
	@echo "# GRMON 2.0 script: Load and run example"         > $@
	@echo "# Example is built for $(BSP)"                   >> $@
	@echo "#"                                               >> $@
	@echo "# Start this script with:"                       >> $@
	@echo "# grmon2> batch $(PROJ).tcl"                     >> $@
	@echo "# grmon2> go 0"                                  >> $@
	@echo "#"                                               >> $@
	@echo "puts \"This example is built for $(BSP)\""       >> $@
	@echo "bp del"                                          >> $@
	@echo "symbols $(PROJ).elf"                             >> $@
	@echo "#run"                                            >> $@

