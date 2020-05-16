include config.mk

# ERC32 applications
ERC32_PROG = rtems-hello rtems-cdtest \
	rtems-tasks rtems-synctrap

# LEON2 applications
LEON2_PROG = rtems-ttcp rtems-hello rtems-cdtest \
	rtems-tasks rtems-synctrap \
	rtems-ttcpw rtems-ttcp rtems-shell \
	rtems-spw-router-pci rtems-tlib rtems-soft-float

# LEON3 BSP applications
LEON3_ALL= \
        rtems-hello rtems-cdtest rtems-tasks rtems-synctrap \
        rtems-ttcpw rtems-ttcp \
        rtems-brm_bm rtems-brm_rt rtems-brm_bc \
        rtems-occan rtems-occan_tx rtems-occan_rx \
        rtems-i2cmst \
	rtems-grcan rtems-grcan_rx rtems-grcan_tx \
	rtems-pci rtems-b1553rt rtems-spi rtems-tlib \
	rtems-gpio rtems-soft-float
LEON3_UP=$(LEON3_ALL) \
	rtems-spwtest_2boards_rx rtems-spwtest_2boards_tx \
	rtems-spwtest_loopback
LEON3_SMP=$(LEON3_ALL)
LEON3_ASMP=$(LEON3_ALL) \
	rtems-mp

# Applications relative the ODIR
ERC32_ODIR=$(addprefix $(ODIR),$(ERC32_PROG))
LEON2_ODIR=$(addprefix $(ODIR),$(LEON2_PROG))
LEON3_UP_ODIR=$(addprefix $(ODIR),$(LEON3_UP))
LEON3_SMP_ODIR=$(addprefix $(ODIR),$(LEON3_SMP))
LEON3_ASMP_ODIR=$(addprefix $(ODIR),$(LEON3_ASMP))

SUBDIRS=1553 gr712 gr740/ampdemo gr740/spw-pci-stress-demo l2cache l4stat \
         memscrub mmu pci/pci-demo spw/grspw-pkt spw/grspw-test spw/rmap \
	 spw/router-demo spw/spwtdp

# Build all samples for all BSPs by default
all: $(BSPS)

clean:
	rm -rf bin/*/*/* bin/erc32/* bin/leon*/* bin/gr*/* bin/ut*/* > /dev/null
	rm -f $(LEON2_PROG) $(LEON3_PROGS) $(LEON3MP_PROGS) > /dev/null
	rm -rf b_*.c *.mkprom *.grmon *.grmon2 *.out *.o tarfile *.srec *.exe *.ali *.o core xdump.s > /dev/null

.PHONY: $(BSPS) $(addsuffix _build,$(BSPS))

erc32_build: $(ERC32_ODIR)
leon2_build: $(LEON2_ODIR)
at697f_build: $(LEON2_ODIR)
leon3_build: $(LEON3_UP_ODIR)
#leon3:
#	$(MAKE) BSP=leon3 ODIR=bin/leon3 leon3_build

leon3_smp_build: $(LEON3_SMP_ODIR)
#leon3_smp:
#	$(MAKE) BSP=leon3_smp ODIR=bin/leon3_smp leon3_smp_build

leon3_sf_build: $(LEON3_UP_ODIR)
leon3_sf_smp_build: $(LEON3_SMP_ODIR)
leon3_flat_build: $(LEON3_UP_ODIR)
leon3_flat_smp_build: $(LEON3_SMP_ODIR)
leon3_std_build: $(LEON3_UP_ODIR)
#leon3_std:
#	$(MAKE) BSP=leon3_std ODIR=bin/leon3_std leon3_std_build

leon3_mp_build: $(LEON3_ASMP_ODIR)
#leon3_mp:
#	$(MAKE) BSP=leon3_mp ODIR=bin/leon3_mp leon3_mp_build

gr712rc_build: $(LEON3_UP_ODIR)
#gr712rc:
#	$(MAKE) BSP=gr712rc ODIR=bin/gr712rc gr712rc_build

gr712rc_smp_build: $(LEON3_SMP_ODIR)
#gr712rc_smp:
#	$(MAKE) BSP=gr712rc_smp ODIR=bin/gr712rc_smp gr712rc_smp_build

gr740_build: $(LEON3_UP_ODIR)
#gr740:
#	$(MAKE) BSP=gr740 ODIR=bin/gr740 gr740_build

gr740_smp_build: $(LEON3_SMP_ODIR)
#gr740_smp:
#	$(MAKE) BSP=gr740_smp ODIR=bin/gr740_smp gr740_smp_build

ut699_build: $(LEON3_UP_ODIR)
#ut699:
#	$(MAKE) BSP=ut699 ODIR=bin/ut699 ut699_build

#ut699e:
#	$(MAKE) BSP=ut700 ODIR=bin/ut700 ut700_build

ut700_build: $(LEON3_UP_ODIR)
#ut700: 
#	$(MAKE) BSP=ut700 ODIR=bin/ut700 ut700_build

$(ODIR)rtems-hello: rtems-hello.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-hello-multiple: rtems-hello-multiple.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

# assume for debugging
$(ODIR)rtems-uart-loopback: rtems-uart-loopback.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-tasks: rtems-tasks.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-irq: rtems-irq.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-synctrap: rtems-synctrap.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-cdtest: rtems-cdtest.cc $(CONFIG_DEPS) | $(ODIR)
	$(CPP) $(CFLAGS) $< -o $@

$(ODIR)rtems-ttcp: rtems-ttcp.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DREAD_TEST_ONLY $< -o $@

$(ODIR)rtems-io: rtems-io.c $(CONFIG_DEPS) $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-ttcpw: rtems-ttcp.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DWRITE_TEST_ONLY $< -o $@

$(ODIR)rtems-pd: rtems-pd.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DREAD_TEST_ONLY $< -o $@

$(ODIR)rtems-soft-float: rtems-soft-float.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

# only LEON3
$(ODIR)rtems-occan: rtems-occan.c occan_lib.h occan_lib.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DTASK_TX -DTASK_RX $< occan_lib.c -o $@

$(ODIR)rtems-occan_tx: rtems-occan.c occan_lib.h occan_lib.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DMULTI_BOARD -DTASK_TX $< occan_lib.c -o $@
	
$(ODIR)rtems-occan_rx: rtems-occan.c occan_lib.h occan_lib.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DMULTI_BOARD -DTASK_RX $< occan_lib.c -o $@

$(ODIR)rtems-spwtest_2boards_rx: rtems-spwtest-2boards.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DTASK_RX $< -o $@
	
$(ODIR)rtems-spwtest_2boards_tx: rtems-spwtest-2boards.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DTASK_TX $< -o $@

$(ODIR)rtems-spwtest_loopback: rtems-spwtest-2boards.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DTASK_TX -DTASK_RX $< -o $@

$(ODIR)rtems-brm_bc: rtems-brm.c brm_lib.c brm_lib.h $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DBRM_BC_TEST $< brm_lib.c -o $@

$(ODIR)rtems-brm_rt: rtems-brm.c brm_lib.c brm_lib.h $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< brm_lib.c -o $@

$(ODIR)rtems-brm_bm: rtems-brm.c brm_lib.c brm_lib.h $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DBRM_BM_TEST $< brm_lib.c -o $@

$(ODIR)rtems-i2cmst: rtems-i2cmst.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-spi: rtems-spi.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

# Used to receive messages from rtems-grcan_tx running on another board
$(ODIR)rtems-grcan_rx: rtems-grcan.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DCANRX_ONLY $< -o $@

# Used to transmit messages to rtems-grcan_rx running on another board
$(ODIR)rtems-grcan_tx: rtems-grcan.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) -DCANTX_ONLY $< -o $@

# This test assumes an external board is responding to the transmitted 
# messages. similar to rtems-canloopback.
$(ODIR)rtems-grcan: rtems-grcan.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

# This test assumes an external board is responding to the transmitted
# messages. similar to rtems-canloopback.
$(ODIR)rtems-grcanfd: rtems-grcanfd.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

# Sets up PCI configuration space and prints out AMBA & PCI device found
$(ODIR)rtems-pci: rtems-pci.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

# Sets up PCI configuration space and prints out AMBA & PCI device found
$(ODIR)rtems-gpio: rtems-gpio.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

# Sets up PCI configuration space and prints out AMBA & PCI device found
$(ODIR)rtems-spw-router-pci: rtems-spw-router-pci.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

# GRPWM 4 channel example application
$(ODIR)rtems-grpwm: rtems-grpwm.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-b1553rt: rtems-b1553rt.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-watchdog: rtems-watchdog.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-shell: rtems-shell.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-flash: rtems-flash.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

$(ODIR)rtems-tlib: rtems-tlib.c $(CONFIG_DEPS) | $(ODIR)
	$(CC) $(CFLAGS) $< -o $@

# AMP Targets 
$(ODIR)rtems-mp-batch-ram.grmon2:ODIR
	@echo "\# GRMON 2.0 script: Load and run RTEMS AMP example" > $@
	@echo "bp del" 				 >> $@
	@echo "load rtems-mp1 cpu0" 		 >> $@
	@echo "ep $(word 1,$(MP_TEXT)) cpu0"	 >> $@
	@echo "stack $(word 1,$(MP_STACK)) cpu0" >> $@
	@echo "load rtems-mp2 cpu1" 		 >> $@
	@echo "ep $(word 2,$(MP_TEXT)) cpu1" 	 >> $@
	@echo "stack $(word 2,$(MP_STACK)) cpu1" >> $@
	@echo "run" 				 >> $@

$(ODIR)rtems-mp-batch-ram.grmon:ODIR
	@echo "\# GRMON 1.1 script: Load and run RTEMS AMP example" > $@
	@echo "cpu act 0" 		    >> $@
	@echo "del" 			    >> $@
	@echo "cpu act 1" 		    >> $@
	@echo "del" 			    >> $@
	@echo "load rtems-mp2" 	    	    >> $@
	@echo "load rtems-mp1" 	    	    >> $@
	@echo "ep $(word 2,$(MP_TEXT))"     >> $@
	@echo "stack $(word 2,$(MP_STACK))" >> $@
	@echo "cpu act 0" 	    	    >> $@
	@echo "ep $(word 1,$(MP_TEXT))"	    >> $@
	@echo "stack $(word 1,$(MP_STACK))" >> $@
	@echo "run" 		    	    >> $@

$(ODIR)rtems-mp1: rtems-mp.c $(CONFIG_DEPS) ODIRrtems-mp-batch-ram.grmon $(ODIR)rtems-mp-batch-ram.grmon2 | $(ODIR)
	$(CC) $(CFLAGS) -qleon3mp -DSHM_START=$(MP_SHM_START) -DSHM_SIZE=$(MP_SHM_SIZE) -DNODE_NUMBER=1 -Wl,-Ttext,$(word 1,$(MP_TEXT)) $< -o $@ 

$(ODIR)rtems-mp2: rtems-mp.c $(CONFIG_DEPS) ODIRrtems-mp-batch-ram.grmon $(ODIR)rtems-mp-batch-ram.grmon2 | $(ODIR)
	$(CC) $(CFLAGS) -qleon3mp -DSHM_START=$(MP_SHM_START) -DSHM_SIZE=$(MP_SHM_SIZE) -DNODE_NUMBER=2 -Wl,-Ttext,$(word 2,$(MP_TEXT)) $< -o $@ 

.PHONY: rtems-mp
rtems-mp: $(ODIR)rtems-mp1 $(ODIR)rtems-mp2 $(ODIR)rtems-mp.mkprom 

# Create PROM images using mkprom2, setup MKPROMFLAGS before use.
$(ODIR)rtems-mp.mkprom: $(ODIR)rtems-mp1 $(ODIR)rtems-mp2ODIR
	@echo "### NOTE: Make sure the MKPROMFLAGS are set to flags that correspond to your target system"
	-$(MKPROM2) $(MP_MKRPROMFLAGS) $(MKPROMFLAGS) -o $@ $^

all_subdirs:
	set -e; \
	for dir in $(SUBDIRS); do \
		cd $$dir; \
		make RCC_TOOLCHAIN=$(RCC_TOOLCHAIN); \
		cd -; \
	done

clean_subdirs:
	set -e; \
	for dir in $(SUBDIRS); do \
		echo Cleaning $$dir; \
		cd $$dir; \
		make clean; \
		cd -; \
	done

include targets.mk
