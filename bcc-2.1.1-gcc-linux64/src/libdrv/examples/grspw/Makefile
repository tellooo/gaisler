###############################################################################
# GRSPW driver example
#
###############################################################################

ALL_EX  += term.elf

include ../common.mk

all: $(ALL_EX)
all: size

$(BINDIR)/term.elf: $(OBJDIR)/term.o $(OBJDIR)/init.o $(OBJDIR)/linenoise.o $(OBJDIR)/grspw_pkt_lib.o | $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) -r $(BINDIR) $(OBJDIR)

include ../target.mk
