include ../../config.mk

INCLUDE+=-I.. -I.

PROGS=rtems-rmap

override CONFIG_DEPS=config*.c

all: $(PROGS)

clean:
	rm -f *.o core rtems-rmap > /dev/null

grspw_pkt_lib.o: ../grspw_pkt_lib.c ../grspw_pkt_lib.h
	$(CC) $(CFLAGS) -c  $< -o $@

spwrouter_custom_config.o: spwrouter_custom_config.c
	$(CC) $(CFLAGS) -c $< -o $@

rmap.o: rmap.c rmap.h
	$(CC) $(CFLAGS) -c $< -o $@

rmap_common.o: rmap_common.c rmap.h
	$(CC) $(CFLAGS) -c $< -o $@

rmap_async.o: rmap_async.c rmap.h
	$(CC) $(CFLAGS) -c $< -o $@

rtems-rmap: rtems-rmap.c spwrouter_custom_config.o grspw_pkt_lib.o rmap.o rmap_common.o rmap_async.o $(CONFIG_DEPS)
	$(CC) $(CFLAGS) $< -o $@ spwrouter_custom_config.o grspw_pkt_lib.o rmap.o rmap_common.o rmap_async.o

include ../../targets.mk
