.PHONY: all
all: $(LIBBCC) $(EXTRA_OBJECTS)

.PHONY: install
install: $(LIBBCC) $(EXTRA_OBJECTS)
	mkdir -p $(BCC_DISTDIR)/$(BSPNAME)/$(MULTI_DIR)/
	$(INSTALL_DATA) $(LIBBCC) $(BCC_DISTDIR)/$(BSPNAME)/$(MULTI_DIR)/
	$(INSTALL_DATA) $(EXTRA_OBJECTS) $(BCC_DISTDIR)/$(BSPNAME)/$(MULTI_DIR)/
	$(INSTALL_DATA) $(EXTRA_DATA) $(BCC_DISTDIR)/$(BSPNAME)/$(MULTI_DIR)/
	cp -rv $(BCC_PATH)/shared/include $(BCC_DISTDIR)/$(BSPNAME)/
	mkdir -p $(BCC_DISTDIR)/$(BSPNAME)/include/bcc
	cp -rv $(BCC_PATH)/bsp/$(BSPNAME)/include/bcc $(BCC_DISTDIR)/$(BSPNAME)/include/

.PHONY: clean
clean:
	rm -rf $(BCC_CLEAN)

