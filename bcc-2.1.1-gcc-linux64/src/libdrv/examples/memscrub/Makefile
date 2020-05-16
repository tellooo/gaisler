ALL_EX  += memscrub.elf

include ../common.mk

all: $(ALL_EX)
all: size

$(OBJDIR)/memscrub.o: msgq.h
$(OBJDIR)/fifo.o: fifo.h
$(OBJDIR)/msgq.o: msgq.h
$(OBJDIR)/msgq.o: fifo.h

$(BINDIR)/memscrub.elf: $(OBJDIR)/memscrub.o $(OBJDIR)/fifo.o $(OBJDIR)/msgq.o | $(BINDIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) -r $(BINDIR) $(OBJDIR)

include ../target.mk
