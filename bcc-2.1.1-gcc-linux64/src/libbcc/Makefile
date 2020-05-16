# if CC is not set or empty use sparc-gaisler-elf-gcc
ifeq (,$(findstring clang,$(CC)))
  override CC := sparc-gaisler-elf-gcc
  MULTILIBS = multilibs_gcc
else
  MULTILIBS = multilibs_llvm
endif

all:
	for d in `dir bsp`; do \
	  if [ -f bsp/$$d/$(MULTILIBS) ] ; \
	  then \
	    for ml in `cat bsp/$$d/$(MULTILIBS)`; do \
	      ml_dir=`echo $$ml | cut '-d;' -f1`; \
	      ml_opt=`echo $$ml | cut '-d;' -f2 | sed 's/@/ -/g'`; \
	      $(MAKE) PATH=$(PATH) -C bsp/$${d} -f bsp.mk BSPNAME=$${d} MULTI_DIR="$${ml_dir}" MULTI_FLAGS="$${ml_opt}" all install || exit 1; \
	    done; \
	  fi \
	done;

