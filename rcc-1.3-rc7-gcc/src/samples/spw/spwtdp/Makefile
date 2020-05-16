include ../../config.mk

INCLUDE+=-I.. -I.

PROGS=spwtdp_example-ngmp spwtdp_example-fpga
OBJS=spwrouter_custom_config.o grspw_pkt_lib.o rmap.o rmap_common.o rmap_async.o spwtdp_lib.o

all: $(PROGS)

clean:
	rm -f *.o core spwtdp_example spwtdp_example-ngmp spwtdp_example-fpga > /dev/null

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

spwtdp_lib.o: spwtdp_lib.c spwtdp_lib.h
	$(CC) $(CFLAGS) -c $< -o $@

spwtdp_example: spwtdp_example.c $(OBJS) $(CONFIG_DEPS)
	$(CC) $(CFLAGS) $< -o $@ $(OBJS)

spwtdp_example-fpga: spwtdp_example.c $(OBJS) $(CONFIG_DEPS)
	$(CC) $(CFLAGS) $(TEST_NGMP) $< -DGR740_SLAVE -DFPGA_MASTER -o $@ $(OBJS)

spwtdp_example-ngmp: spwtdp_example.c $(OBJS) $(CONFIG_DEPS)
	$(CC) $(CFLAGS) $(TEST_NGMP) $< -DGR740_MASTER -DFPGA_SLAVE -o $@ $(OBJS)

include ../../targets.mk
