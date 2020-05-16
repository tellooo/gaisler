include ../../config.mk

INCLUDE+=-I.. -I.

PROGS=test

TEST_NGMP=
TEST_LEON3=
ifeq ($(strip $(CPU)),ngmp)
	TEST_NGMP+=-DTEST_NGMP -DHOST_NGMP -DTARGET_NGMP
endif

ifeq ($(strip $(CPU)),ngmp_smp)
	TEST_NGMP+=-DTEST_NGMP -DHOST_NGMP -DTARGET_NGMP
endif

override CONFIG_DEPS=config*.c

all: $(PROGS)

clean:
	rm -f *.o core test test_pci test_pci_rtr test_pci_only > /dev/null

mmu_setup.o: mmu_setup.c
	$(CC) $(CFLAGS) -c $< -o $@

spwrouter_custom_config.o: spwrouter_custom_config.c
	$(CC) $(CFLAGS) -c $< -o $@

grspw_pkt_lib.o: ../grspw_pkt_lib.c ../grspw_pkt_lib.h
	$(CC) $(CFLAGS) -c $< -o $@

spwlib.o: spwlib.c spwlib.h
	$(CC) $(CFLAGS) -c $< -o $@

pci_test.o: pci_test.c pci_test.h
	$(CC) $(CFLAGS) $(TEST_NGMP) -c $< -o $@

# GR740/LEON4-N2X router with 8 SpW ports and 4 AMBA ports. 4 AMBA ports used.
test: test.c test_rtems.c pci_test.o spwlib.o spwrouter_custom_config.o $(if $(TEST_NGMP),mmu_setup.o) grspw_pkt_lib.o $(CONFIG_DEPS)
	$(CC) $(CFLAGS) $(TEST_NGMP) $< -o $@ pci_test.o spwlib.o spwrouter_custom_config.o $(if $(TEST_NGMP),mmu_setup.o) grspw_pkt_lib.o

test_pci_only: test_pci_only.c pci_test.o $(if $(TEST_NGMP),mmu_setup.o) $(CONFIG_DEPS)
	$(CC) $(CFLAGS) $(TEST_NGMP) $< -o $@ pci_test.o $(if $(TEST_NGMP),mmu_setup.o)

