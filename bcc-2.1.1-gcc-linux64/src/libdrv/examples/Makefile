###############################################################################
# Examples
#
###############################################################################

ROOTDIR := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

DSTDIR = $(ROOTDIR)
BINDIR = $(abspath $(DSTDIR)/bin)
OBJDIR = $(abspath $(DSTDIR)/obj)
BSP=leon3

export BSP 

ALL_EX += ahbstat/ahbstat-isr.elf
ALL_EX += grcan/term.elf
ALL_EX += grcan/trans.elf
ALL_EX += grspw/term.elf
ALL_EX += uart/uart.elf
ALL_EX += spi/spi.elf
ALL_EX += gr1553b/gr1553bcbm.elf
ALL_EX += gr1553b/gr1553rtbm.elf
ALL_EX += gr1553b/gr1553bm.elf
ALL_EX += i2cmst/scan.elf
ALL_EX += i2cmst/eeprom.elf
ALL_EX += memprot/memprot.elf
ALL_EX += memscrub/memscrub.elf
ALL_EX += clkgate/clkgate.elf

ALL_DIR = $(patsubst %/,%,$(sort $(dir $(ALL_EX))))

all: $(ALL_EX)

.PHONY: $(ALL_EX) $(ALL_DIR)

# Build individual example
$(ALL_EX): %.elf:
	$(MAKE) -C $(@D) BINDIR=$(BINDIR)/$(@D) OBJDIR=$(OBJDIR)/$(@D) $(@F)

# Build all examples in a folder
$(ALL_DIR):
	$(MAKE) -C $@ BINDIR=$(BINDIR)/$@ OBJDIR=$(OBJDIR)/$@ all

test:
	@echo $(ALL_DIR)

help:
	@echo "Available targets:"; \
	echo "  all"; \
	echo "  clean"; \
	for t in $(ALL_DIR) $(ALL_EX); do echo "  $$t"; done

clean:
	$(RM) -r $(abspath $(BINDIR)) $(abspath $(OBJDIR))
