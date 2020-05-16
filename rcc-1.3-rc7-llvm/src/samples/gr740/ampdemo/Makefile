include ../../config.mk

all: smp-tick-cpu0 sp-tick-cpu3 smp-tick-cpu1 sp-tick-cpu0

override CFLAGS=-Wall -Wextra -Os -Werror -mcpu=leon3 -g -msoft-float
SPFLAGS=-qbsp=gr740 -Wl,-Ttext,0x01000000
SMPFLAGS=-qbsp=gr740_smp

smp-tick-cpu0: smp-tick.c
	$(CC) $(CFLAGS) $(SMPFLAGS) -DCFG_ISMASTER=1 $< -o $@
sp-tick-cpu3: sp-tick.c
	$(CC) $(CFLAGS) $(SPFLAGS) -DCFG_ISMASTER=0 $< -o $@

smp-tick-cpu1: smp-tick.c
	$(CC) $(CFLAGS) $(SMPFLAGS) -DCFG_ISMASTER=0 $< -o $@
sp-tick-cpu0: sp-tick.c
	$(CC) $(CFLAGS) $(SPFLAGS) -DCFG_ISMASTER=1 $< -o $@

clean:
	@rm -f smp-tick-cpu0
	@rm -f smp-tick-cpu1
	@rm -f sp-tick-cpu3
	@rm -f sp-tick-cpu0

include ../../targets.mk
