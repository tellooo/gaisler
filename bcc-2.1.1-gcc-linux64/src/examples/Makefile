# variables

SHELL = /bin/bash

PREFIX=sparc-gaisler-elf-
CC = $(PREFIX)gcc
CFLAGS = -g -O3

EXAMPLES         =
EXAMPLES        += hello.elf
EXAMPLES        += mainarg.elf
EXAMPLES        += dhrystone.elf
EXAMPLES        += stanford.elf
EXAMPLES        += whetstone.elf
EXAMPLES        += paranoia.elf
EXAMPLES        += prime.elf
EXAMPLES        += ambapp.elf
EXAMPLES        += bcc_pil.elf
EXAMPLES        += fast_interrupt.elf
EXAMPLES        += bcc_fpu_state.elf
EXAMPLES        += bcc_amp.elf
EXAMPLES        += timestamp.elf

ifeq ($(BSPS),)
# Try to find BSPS given the examples config directory
BSPS := $(notdir $(wildcard bspopts/*))
endif

ifeq ($(DESTDIR),)
DESTDIR = .
endif
BINDIR = bin

FLAGS_TO_PASS = \
	"CC=$(CC)" \
	"CFLAGS=$(CFLAGS)" \
	"AS=$(AS)" \
	"LD=$(LD)" \
	"DESTDIR=$(DESTDIR)"

MAKEFLAGS=--no-print-directory

# targets
all:
	@$(MAKE) $(FLAGS_TO_PASS) multi-do DO="build"

clean:
	rm -rf $(BINDIR)
	rm -f *.elf

.PHONY: build
build: $(addprefix $(DESTDIR)/,$(EXAMPLES))
	@true

$(DESTDIR):
	@mkdir -p $@

# For each BSP, there are several compiler options/output directory
# descriptions, in the file bspopts/<BSP>/opts_<TOOLCHAIN>.
multi-do:
	@for bsp in $(BSPS); do \
		for i in `cat bspopts/$${bsp}/opts_gcc`; do \
			dir="$${bsp}/"; \
			dir+=`echo $$i | sed -e 's/;.*$$//'`; \
			flags="-qbsp=$${bsp} ";\
			flags+=`echo $$i | sed -e 's/^[^;]*;//' -e 's/@/ -/g'`; \
			printf "BSP:%-10s dir:%-20s flags:$${flags}\n" $${bsp} $${dir} > /dev/null; \
			$(MAKE) $(FLAGS_TO_PASS) \
				CFLAGS="$(CFLAGS) $${flags}" \
				CXXFLAGS="$(CXXFLAGS) $${flags}" \
				LDFLAGS="$(LDFLAGS) $${flags}" \
				DESTDIR="$(BINDIR)/$${dir}" \
				BSP="$${bsp}" \
				$(DO) \
			|| exit 1;  \
		done; \
	done

$(DESTDIR)/hello.elf: hello/hello.c | $(DESTDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(DESTDIR)/mainarg.elf: mainarg/mainarg.c | $(DESTDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(DESTDIR)/dhrystone.elf: dhrystone/dhry_1.c dhrystone/dhry_2.c | $(DESTDIR)
	$(CC) $(CFLAGS) -Wno-implicit-int -Wno-implicit-function-declaration $^ -o $@

ifeq ($(BSP),gr716)
# GR716 on-chip Data RAM is to small, so force external RAM linker script.
$(DESTDIR)/stanford.elf: stanford/stanford.c | $(DESTDIR)
	$(CC) $(CFLAGS) -Wno-implicit-int -T linkcmds-ext $^ -o $@
else
$(DESTDIR)/stanford.elf: stanford/stanford.c | $(DESTDIR)
	$(CC) $(CFLAGS) -Wno-implicit-int $^ -o $@
endif

$(DESTDIR)/whetstone.elf: whetstone/whetstone.c | $(DESTDIR)
	$(CC) $(CFLAGS) $^ -o $@ -lm

$(DESTDIR)/paranoia.elf: paranoia/paranoia.c | $(DESTDIR)
	$(CC) $(CFLAGS) $^ -o $@ -lm

ifeq ($(BSP),gr716)
# Force external RAM linker script.
$(DESTDIR)/prime.elf: prime/prime.cc | $(DESTDIR)
	$(CC) $(CFLAGS) -T linkcmds-ext $^ -o $@ -lstdc++
else
$(DESTDIR)/prime.elf: prime/prime.cc | $(DESTDIR)
	$(CC) $(CFLAGS) $^ -o $@ -lstdc++
endif

$(DESTDIR)/timestamp.elf: timestamp/main.c | $(DESTDIR)
	$(CC) $(CFLAGS) -Wall -Wextra -pedantic -std=c99 $^ -ldrv -o $@

$(DESTDIR)/ambapp.elf: ambapp/ambapp.c | $(DESTDIR)
	$(CC) $(CFLAGS) -std=c99 $^ -o $@

$(DESTDIR)/bcc_pil.elf: bcc_pil/bcc_pil.c | $(DESTDIR)
	$(CC) $(CFLAGS) -std=c99 $^ -o $@

$(DESTDIR)/fast_interrupt.elf: fast_interrupt/myinthandler.S fast_interrupt/main.c | $(DESTDIR)
	$(CC) $(CFLAGS) -std=c99 $^ -o $@

$(DESTDIR)/bcc_fpu_state.elf: bcc_fpu_state/bcc_fpu_state.c bcc_fpu_state/ffun.c bcc_fpu_state/nest_force.s | $(DESTDIR)
	$(CC) $(CFLAGS) -std=c99 $^ -o $@

.PHONY: $(DESTDIR)/bcc_amp.elf

ifeq ($(BSP),gr712rc)
AMP_NODES = 2
AMP_TEXT =  0x40001000 0x40100000
AMP_STACK = 0x400fff00 0x401fff00
$(DESTDIR)/bcc_amp.elf: $(DESTDIR)/bcc_amp0.elf $(DESTDIR)/bcc_amp1.elf $(DESTDIR)/bcc_amp.grmon2
$(DESTDIR)/bcc_amp0.elf: bcc_amp/bcc_amp.c | $(DESTDIR)
	$(CC) $(CFLAGS) -std=c99 -DAMP_NODES=$(AMP_NODES) -DNODE_INDEX=0 -Wl,-Ttext,$(word 1, $(AMP_TEXT)) -Wl,-Map=$@.map $^ -o $@
else ifeq ($(BSP), gr740)
AMP_NODES = 2
AMP_TEXT =  0x00001000 0x00100000 0x00200000 0x00300000
AMP_STACK = 0x000fff00 0x001fff00 0x002fff00 0x003fff00
$(DESTDIR)/bcc_amp.elf: $(DESTDIR)/bcc_amp0.elf $(DESTDIR)/bcc_amp1.elf $(DESTDIR)/bcc_amp.grmon2
$(DESTDIR)/bcc_amp0.elf: bcc_amp/bcc_amp.c | $(DESTDIR)
	$(CC) $(CFLAGS) -std=c99 -DAMP_NODES=$(AMP_NODES) -DNODE_INDEX=0 -Wl,-Ttext,$(word 1, $(AMP_TEXT)) -Wl,-Map=$@.map $^ -o $@
else
AMP_NODES = 1
$(DESTDIR)/bcc_amp.elf: $(DESTDIR)/bcc_amp0.elf
$(DESTDIR)/bcc_amp0.elf: bcc_amp/bcc_amp.c | $(DESTDIR)
	$(CC) $(CFLAGS) -std=c99 -DAMP_NODES=$(AMP_NODES) -DNODE_INDEX=0 -Wl,-Map=$@.map $^ -o $@
endif

$(DESTDIR)/bcc_amp1.elf: bcc_amp/bcc_amp.c | $(DESTDIR)
	$(CC) $(CFLAGS) -std=c99 -DAMP_NODES=$(AMP_NODES) -DNODE_INDEX=1 -Wl,-Ttext,$(word 2, $(AMP_TEXT)) -Wl,-Map=$@.map $^ -o $@

$(DESTDIR)/bcc_amp.grmon2: $(DESTDIR)
	@echo "# GRMON 2.0 script: Load and run BCC AMP example" > $@
	@echo "# Start this script with:"               >> $@
	@echo "# grmon2> batch bcc_amp.grmon2"          >> $@
	@echo "bp del"                                  >> $@
	@echo "load bcc_amp0.elf cpu0"                  >> $@
	@echo "stack $(word 1,$(AMP_STACK)) cpu0"       >> $@
	@echo "load bcc_amp1.elf cpu1"                  >> $@
	@echo "stack $(word 2,$(AMP_STACK)) cpu1"       >> $@
	@echo "run"                                     >> $@

