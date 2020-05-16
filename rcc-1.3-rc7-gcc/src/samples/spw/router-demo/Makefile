include ../../config.mk

# Supported BSPs for this tests.
BSPS=leon3 leon3_smp gr712rc gr712rc_smp gr740 gr740_smp
ifneq ($(RCC_TOOLCHAIN),llvm)
  BSPS+=ut699 ut700
endif

# AMBA ports systems: leon3 leon3_smp gr712rc gr712rc_smp ut699 ut700
GRSPW2_DUAL=$(addprefix $(ODIR),test_dual)
# PCI systems: leon3 leon3_smp gr740 gr740_smp ut700
#GRSPW2_PCI=$(addprefix $(ODIR),test_pci)
# Router Systems: leon3 leon3_smp gr740 gr740_smp
GRSPW2_RTR=$(addprefix $(ODIR),test_rtr)

INCLUDE+=-I.. -I.
CFLAGS+=$(INCLUDE)

TEST_NGMP=
TEST_LEON3=
ifeq ($(strip $(BSP)),gr740)
	TEST_NGMP+=-DTEST_NGMP
endif

ifeq ($(strip $(BSP)),gr740_smp)
	TEST_NGMP+=-DTEST_NGMP
endif

override CONFIG_DEPS=config*.c

.PHONY: all clean
all:
	for bsp in $(BSPS); do $(MAKE) $$bsp BSP=$$bsp; done

clean:
	rm -f *.o core test test_pci test_pci_rtr > /dev/null
	rm -rf bin/

# Configure which systems/BSPs are compatible with the tests 
leon3_build: $(GRSPW2_DUAL) $(GRSPW2_RTR)
leon3_smp_build: $(GRSPW2_DUAL) $(GRSPW2_RTR)
gr712rc_build: $(GRSPW2_DUAL)
gr712rc_smp_build: $(GRSPW2_DUAL)
gr740_build: $(GRSPW2_RTR)
gr740_smp_build: $(GRSPW2_RTR)
ut699_build: $(GRSPW2_DUAL)
ut700_build: $(GRSPW2_DUAL)

$(ODIR)mmu_setup.o: mmu_setup.c | $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)spwrouter_custom_config.o: spwrouter_custom_config.c | $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)grspw_pkt_lib.o: ../grspw_pkt_lib.c ../grspw_pkt_lib.h | $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)spwlib.o: spwlib.c spwlib.h | $(ODIR)
	$(CC) $(CFLAGS) -c $< -o $@

# GR740/LEON4-N2X router with 8 SpW ports and 4 AMBA ports. 4 AMBA ports used.
$(ODIR)test_rtr: test.c test_rtems.c $(ODIR)spwlib.o $(ODIR)spwrouter_custom_config.o \
		 $(if $(TEST_NGMP),$(ODIR)mmu_setup.o) $(ODIR)grspw_pkt_lib.o $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $(TEST_NGMP) $< -o $@ $(ODIR)spwlib.o $(ODIR)spwrouter_custom_config.o $(if $(TEST_NGMP),$(ODIR)mmu_setup.o) $(ODIR)grspw_pkt_lib.o

# Loopback between two SpW connectors without SpW router. Compatible with GR712RC/UT699E/UT700/etc.
$(ODIR)test_dual: test.c test_rtems.c $(ODIR)spwlib.o $(ODIR)spwrouter_custom_config.o \
		$(if $(TEST_NGMP),$(ODIR)mmu_setup.o) $(ODIR)grspw_pkt_lib.o $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $(TEST_LEON3) -DDUAL_LOOPBACK $< -o $@ $(ODIR)spwlib.o $(ODIR)spwrouter_custom_config.o $(if $(TEST_NGMP),$(ODIR)mmu_setup.o) $(ODIR)grspw_pkt_lib.o

$(ODIR)test_pci: test_pci.c $(ODIR)spwlib.o $(ODIR)spwrouter_custom_config.o \
		$(if $(TEST_NGMP),$(ODIR)mmu_setup.o) $(ODIR)grspw_pkt_lib.o $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $(TEST_NGMP) $< -o $@ $(ODIR)spwlib.o $(ODIR)spwrouter_custom_config.o $(if $(TEST_NGMP),$(ODIR)mmu_setup.o) $(ODIR)grspw_pkt_lib.o

$(ODIR)test_pci_rtr: test_pci_rtr.c spwlib.o spwrouter_custom_config.o \
		$(if $(TEST_NGMP),$(ODIR)mmu_setup.o) $(ODIR)grspw_pkt_lib.o $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $(TEST_NGMP) $< -o $@ $(ODIR)spwlib.o $(ODIR)spwrouter_custom_config.o $(if $(TEST_NGMP),$(ODIR)mmu_setup.o) $(ODIR)grspw_pkt_lib.o


include ../../targets.mk
