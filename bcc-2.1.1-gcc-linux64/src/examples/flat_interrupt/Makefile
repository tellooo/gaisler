PREFIX=sparc-gaisler-elf-
CC = $(PREFIX)gcc
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size
CCOPT = -O2
#BSP = gr716
BSP = leon3
CFLAGS = -mflat -qbsp=$(BSP) -mcpu=leon3v7 -qnano -std=c99 -Wall -Wextra -pedantic $(CCOPT) $(LDFLAGS)
#CFLAGS += -qsvt

PROJ=ex
SOURCES  =
SOURCES += main.c
SOURCES += fl_isr.c
SOURCES += ll.S

all: $(PROJ).elf $(PROJ).dis $(PROJ).tcl

$(PROJ).elf: $(SOURCES)
	$(CC) $(CFLAGS) -Wl,-Map=$(PROJ).map $^ -o $@

$(PROJ).dis: $(PROJ).elf
	$(OBJDUMP) -d $^ > $@

size: $(PROJ).elf
	$(SIZE) $^

clean:
	rm -f $(PROJ).elf $(PROJ).map $(PROJ).dis $(PROJ).tcl

$(PROJ).tcl: $(PROJ).elf
	@echo "# GRMON script: Load and run example"             > $@
	@echo "#"                                               >> $@
	@echo "# Start this script with:"                       >> $@
	@echo "# grmon3> batch $(PROJ).tcl"                     >> $@
	@echo "# grmon3> run"                                   >> $@
	@echo "#"                                               >> $@
	@echo "puts \"This example is built for $(BSP)\""       >> $@
	@echo "load $(PROJ).elf"                                >> $@
	@echo "run"                                             >> $@

