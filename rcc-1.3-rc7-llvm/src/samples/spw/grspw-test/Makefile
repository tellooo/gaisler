include ../../config.mk

INCLUDE+=-I.. -I.

PROGS=test

all: $(PROGS)

clean:
	rm -f *.o core test > /dev/null

spwrouter_custom_config.o: spwrouter_custom_config.c
	$(CC) $(CFLAGS) -c $< -o $@

grspw_pkt_lib.o: ../grspw_pkt_lib.c ../grspw_pkt_lib.h
	$(CC) $(CFLAGS) -c $< -o $@

test: test.c spwrouter_custom_config.o grspw_pkt_lib.o $(CONFIG_DEPS)
	$(CC) $(CFLAGS) $< -o $@ spwrouter_custom_config.o grspw_pkt_lib.o

include ../../targets.mk
