ifeq (,$(findstring clang,$(CC)))
  override CC := sparc-gaisler-elf-gcc
endif

BUILDDIR ?= build
DESTDIR ?= dist
JOPT ?= 4
all:
	for ml in `$(CC) -print-multi-lib`; do \
	  ml_dir=`echo $$ml | cut '-d;' -f1`; \
	  ml_opt=`echo $$ml | cut '-d;' -f2 | sed 's/@/ -/g'`; \
	  $(MAKE) -j $(JOPT) PATH=$(PATH) -f Makefile.lib BUILDDIR=$(BUILDDIR) DESTDIR=$(DESTDIR) ml_dir="$${ml_dir}" ml_opt="$${ml_opt}" || exit 1; \
	done
	@mkdir -p $(DESTDIR)/include
	@cp -r src/include/* $(DESTDIR)/include/
	@cp -r src/osal/bcc2/include/* $(DESTDIR)/include/

