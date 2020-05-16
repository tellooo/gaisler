
.PHONY: $(ALL_EX)
$(ALL_EX): %.elf: $(BINDIR)/%.elf

$(BINDIR)/%.elf: $(OBJDIR)/%.o | $(BINDIR)
	$(CC) $(CFLAGS) $(filter %.o,$^) -o $@ $(LDFLAGS) $(LDLIBS)

.PRECIOUS: $(OBJDIR)/%.o
$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BINDIR) $(OBJDIR):
	mkdir -p $@

size: $(addprefix $(BINDIR)/,$(ALL_EX))
	$(SIZE) $^

$(OBJDIR)/linenoise.o: $(ROOTDIR)/linenoise.c $(ROOTDIR)/linenoise.h
	$(CC) $(CFLAGS) -c $< -o $@

