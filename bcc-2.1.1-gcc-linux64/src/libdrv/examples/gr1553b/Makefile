###############################################################################
# GRCAN driver example
#
###############################################################################

ALL_EX  += gr1553bcbm.elf
ALL_EX  += gr1553rtbm.elf
ALL_EX  += gr1553bm.elf

include ../common.mk

all: $(ALL_EX)
all: size

$(BINDIR)/gr1553bcbm.elf: $(OBJDIR)/gr1553bcbm.o $(OBJDIR)/bm_logger.o | $(BINDIR)

$(BINDIR)/gr1553rtbm.elf: $(OBJDIR)/gr1553rtbm.o $(OBJDIR)/bm_logger.o | $(BINDIR)

$(BINDIR)/gr1553bm.elf: $(OBJDIR)/gr1553bm.o $(OBJDIR)/bm_logger.o | $(BINDIR)

$(OBJDIR)/gr1553bcbm.o: gr1553bcbm.c bm_logger.h pnp1553.h | $(OBJDIR) 

$(OBJDIR)/gr1553rtbm.o: gr1553rtbm.c bm_logger.h pnp1553.h | $(OBJDIR) 

$(OBJDIR)/gr1553bm.o: gr1553bm.c bm_logger.h | $(OBJDIR) 

$(OBJDIR)/bm_logger.o: bm_logger.h | $(OBJDIR) 

clean:
	$(RM) -r $(BINDIR) $(OBJDIR)

include ../target.mk
