# variables

SHELL = /bin/bash

PREFIX=sparc-gaisler-elf-
CC = $(PREFIX)gcc
CCOPT=-O2
INCFLAGS = -I.
CFLAGS = -qbsp=$(BSP) -mcpu=leon3 -std=c11 -Wall -Wextra -pedantic $(CCOPT) $(INCFLAGS)

PROJ=ampmsg

SOURCES = main.c slist.c mutex.c msg.c
ELFNAME = $(PROJ).elf

AMP_COMM_SIZE = 0x1000
ifeq ($(BSP),gr712rc)
 AMP_NODES = 2
 AMP_COMM =  0x40001000
 AMP_TEXT =  0x40002000 0x40100000
 AMP_STACK = 0x400fff00 0x401fff00
 CFLAGS += -mfix-gr712rc
all: $(PROJ).tcl
else ifeq ($(BSP), gr740)
 AMP_NODES = 2
 AMP_COMM =  0x00001000
 AMP_TEXT =  0x00002000 0x00100000 0x00200000 0x00300000
 AMP_STACK = 0x000fff00 0x001fff00 0x002fff00 0x003fff00
all: $(PROJ).tcl
else
all:
	@echo "Use 'make BSP=gr712rc' or 'make BSP=gr740'"
endif

$(PROJ)0.elf: $(SOURCES) node0.c
	$(CC) $(CFLAGS) -DAMP_COMM=$(AMP_COMM) -DAMP_NODES=$(AMP_NODES) -DNODE_INDEX=0 -Wl,-Ttext,$(word 1, $(AMP_TEXT)) $^ -o $@
$(PROJ)1.elf: $(SOURCES) node1.c
	$(CC) $(CFLAGS) -DAMP_COMM=$(AMP_COMM) -DAMP_NODES=$(AMP_NODES) -DNODE_INDEX=1 -Wl,-Ttext,$(word 2, $(AMP_TEXT)) $^ -o $@

clean:
	rm -f *.elf $(PROJ).tcl

$(PROJ).tcl: $(PROJ)0.elf $(PROJ)1.elf
	@echo "# GRMON 2.0 script: Load and run BCC AMP example" > $@
	@echo "# Example is built for $(BSP)"                   >> $@
	@echo "#"                                               >> $@
	@echo "# Start this script with:"                       >> $@
	@echo "# grmon2> batch $(PROJ).tcl"                     >> $@
	@echo "# grmon2> run"                                   >> $@
	@echo "#"                                               >> $@
	@echo "puts \"This example is built for $(BSP)\""       >> $@
	@echo "bp del"                                          >> $@
	@echo "wash $(AMP_COMM) [expr {$(AMP_COMM)+0xfff}]"     >> $@
	@echo "load $(PROJ)0.elf cpu0"                          >> $@
	@echo "silent stack $(word 1,$(AMP_STACK)) cpu0"        >> $@
	@echo "load $(PROJ)1.elf cpu1"                          >> $@
	@echo "silent stack $(word 2,$(AMP_STACK)) cpu1"        >> $@
	@echo "stack"                                           >> $@
	@echo "ep"                                              >> $@

