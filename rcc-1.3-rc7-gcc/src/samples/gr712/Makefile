# --------------------------------------------------------------------------
#  --  This file is a part of COBHAM RESEARCH source code.
#  --  Copyright (C) 2015, Cobham Gaisler AB - all rights reserved.
#  --
#  -- ANY USE OR REDISTRIBUTION IN PART OR IN WHOLE MUST BE HANDLED IN
#  -- ACCORDANCE WITH THE GAISLER LICENSE AGREEMENT AND MUST BE APPROVED
#  -- IN ADVANCE IN WRITING.
#  --
#  -- BY DEFAULT, DISTRIBUTION OR DISCLOSURE IS NOT PERMITTED.
#  -------------------------------------------------------------------------- 
#
include ../config.mk

PROGS= rtems-ascs rtems-satcan rtems-slink

all: gr712rc

gr712rc_build: $(PROGS)

rtems-ascs: rtems-ascs.c $(CONFIG_DEPS) $(TOOLCHAIN_DEP)
	$(CC) $(CFLAGS) -o $@ $<

rtems-satcan: rtems-satcan.c occan_lib.o $(CONFIG_DEPS) $(TOOLCHAIN_DEP)
	$(CC) $(CFLAGS) -o $@ $< occan_lib.o

occan_lib.o: ../occan_lib.c ../occan_lib.h $(TOOLCHAIN_DEP)
	$(CC) $(CFLAGS) -c  $< -o $@

rtems-slink: rtems-slink.c $(CONFIG_DEPS) $(TOOLCHAIN_DEP)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	-rm -f *.o $(PROGS) > /dev/null

include ../targets.mk
