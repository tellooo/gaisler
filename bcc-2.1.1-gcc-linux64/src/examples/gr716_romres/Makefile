PREFIX=sparc-gaisler-elf-
CC = $(PREFIX)gcc
OBJDUMP = $(PREFIX)objdump
CCOPT = -Os
LDFLAGS = -T linkcmds-extprom
BSP = gr716
CFLAGS = -qbsp=$(BSP) -mcpu=leon3 -qnano -qsvt -std=c99 -Wall -Wextra -pedantic $(CCOPT) $(LDFLAGS)

PROJ=romres
SOURCES  = 
SOURCES += main.c
SOURCES += my_svt_tables.S

all: $(PROJ).elf $(PROJ).dis

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

