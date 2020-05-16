###############################################################################
# GRCAN driver example
#
###############################################################################

ALL_EX  += term.elf
ALL_EX  += trans.elf

include ../common.mk

all: $(ALL_EX)
all: size

$(BINDIR)/term.elf: $(OBJDIR)/term.o $(OBJDIR)/linenoise.o | $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) -r $(BINDIR) $(OBJDIR)

include ../target.mk
