include ../config.mk

# Location of Driver-Manager Configuration Files
CFGDIR=..

# Build flags
CFLAGS=-g3 -O0 -Wall
LIBS=


# CPU Dependent compiler config
LEON2= -qbsp=leon2
LEON3= -qbsp=leon3

# Supported BSPs for this tests.
# ut700/699e and GR740 has GR1533B
BSPS=leon3 gr740
ifneq ($(RCC_TOOLCHAIN),llvm)
  BSPS+=ut700
endif

.PHONY:rtems-gr1553bm rtems-gr1553bcbm rtems-gr1553rtbm ethclient test1

GR1553B_DEMOS=$(addprefix $(ODIR), rtems-gr1553bm rtems-gr1553bcbm rtems-gr1553rtbm)
GR1553B_AT697_DEMOS=rtems-gr1553bcbm-leon2 rtems-gr1553rtbm-leon2 rtems-gr1553bm-leon2

.PHONY: all clean ethclient test1
all:
	for bsp in $(BSPS); do $(MAKE) $$bsp BSP=$$bsp; done


leon3_build: $(GR1553B_DEMOS)
ut700_build: $(GR1553B_DEMOS)
gr740_build: $(GR1553B_DEMOS)
at697f_build: $(GR1553B_AT697_DEMOS)

BMOBJ=$(ODIR)ethsrv.o $(ODIR)bm_logger.o

# BC and BM
$(ODIR)rtems-gr1553bcbm: rtems-gr1553bcbm.c $(BMOBJ) bm_logger.h  $(ODIR)time.o  time.h | $(ODIR)
	$(CC) $(CFLAGS) -I$(CFGDIR) -DSOFT_EXTTRIG_ENABLE rtems-gr1553bcbm.c -o $@ $(BMOBJ)  $(ODIR)time.o $(LIBS)
$(ODIR)rtems-gr1553bcbm-leon2: rtems-gr1553bcbm.c $(BMOBJ) bm_logger.h $(ODIR)time.o time.h | $(ODIR)
	$(CC) $(CFLAGS) -I$(CFGDIR) -DSOFT_EXTTRIG_ENABLE -DAMBA_OVER_PCI rtems-gr1553bcbm.c -o $@ $(BMOBJ)  $(ODIR)time.o $(LIBS)

## SPWCUC driver not available in RCC-1.3 yet so these targets are disabled until available.
#rtems-gr1553bcbm-exttrig: rtems-gr1553bcbm.c time.c
#	$(CC) $(CFLAGS) -c time.c
#	$(CC) $(CFLAGS) -I$(CFGDIR) rtems-gr1553bcbm.c -o rtems-gr1553bcbm-exttrig time.o $(LIBS)
#rtems-gr1553bcbm-leon2-exttrig: rtems-gr1553bcbm.c time.c
#	$(CC) $(CFLAGS) -c time.c
#	$(CC) $(CFLAGS) -I$(CFGDIR) -DAMBA_OVER_PCI rtems-gr1553bcbm.c -o rtems-gr1553bcbm-leon2-exttrig time.o $(LIBS)


# Linux TCP/IP client talking with BM in rtems-gr1553bcbm example
#
# Note that if only builds when the rtems-gr1553bcbm example has been configured
# to support TCP/IP server. See config_bm.h
ethclient:
	gcc -Wall -g3 -O0 ethclient.c -o ethclient

$(ODIR)time.o: time.c time.h | $(ODIR)
	$(CC) $(CFLAGS) -I$(CFGDIR) $(LEON3) -c $< -o $@

$(ODIR)bm_logger.o: bm_logger.c bm_logger.h ethsrv.h | $(ODIR)
	$(CC) $(CFLAGS) -I$(CFGDIR) $(LEON3) -c $< -o $@

$(ODIR)ethsrv.o: ethsrv.c ethsrv.h | $(ODIR)
	$(CC) $(CFLAGS) -I$(CFGDIR) $(LEON3) -c $< -o $@

# RT and BM
$(ODIR)rtems-gr1553rtbm: rtems-gr1553rtbm.c $(BMOBJ) bm_logger.h | $(ODIR)
	$(CC) $(CFLAGS) -I$(CFGDIR) $(LEON3) rtems-gr1553rtbm.c -o $@ $(BMOBJ) $(LIBS)
$(ODIR)rtems-gr1553rtbm-leon2: rtems-gr1553rtbm.c $(BMOBJ) bm_logger.h | $(ODIR)
	$(CC) $(CFLAGS) -I$(CFGDIR) $(LEON2) -DAMBA_OVER_PCI rtems-gr1553rtbm.c -o $@ $(BMOBJ) $(LIBS)

# BM only
$(ODIR)rtems-gr1553bm: rtems-gr1553bm.c $(BMOBJ) bm_logger.h | $(ODIR)
	$(CC) $(CFLAGS) -I$(CFGDIR) $(LEON3) rtems-gr1553bm.c -o $@ $(BMOBJ) $(LIBS)
$(ODIR)rtems-gr1553bm-leon2: rtems-gr1553bm.c $(BMOBJ) bm_logger.h | $(ODIR)
	$(CC) $(CFLAGS) -I$(CFGDIR) $(LEON2) -DAMBA_OVER_PCI rtems-gr1553bm.c -o $@ $(BMOBJ) $(LIBS)

# A AT697+GR-RASTA-IO (BC) TimeMaster distibuting time via SpaceWire to 
# A LEON3 GR-RASTA-105 TimeSlave (BC). The GR1553B BC core distributes time onto a 1553 bus to
# A Standard LEON3 with GR1553B RT core acting as a RT Time Slave
test1: $(BMOBJ)
	# The Two BC Apps
	mkdir -p test1
	$(CC) $(CFLAGS) -I$(CFGDIR) $(LEON3) -DTIME_SYNC_MANAGEMENT \
		rtems-gr1553bcbm.c -o test1/rtems-gr1553bcbm-test1-leon3  $(ODIR)time.o $(BMOBJ) $(LIBS)
	$(CC) $(CFLAGS) -I$(CFGDIR) $(LEON2) -DAMBA_OVER_PCI -DTIME_SYNC_MANAGEMENT -DTIME_SYNC_MASTER \
		rtems-gr1553bcbm.c -o test1/rtems-gr1553bcbm-test1-leon2  $(ODIR)time.o $(BMOBJ) $(LIBS)
	# The RT Application
	$(CC) $(CFLAGS) -I$(CFGDIR) $(LEON3) rtems-gr1553rtbm.c -o test1/rtems-gr1553rtbm-test1-leon3 $(BMOBJ) $(LIBS)

clean:
	rm -rf \
		*.o \
		rtems-gr1553bm \
		rtems-gr1553bm-leon2 \
		rtems-gr1553rtbm \
		rtems-gr1553rtbm-leon2 \
		rtems-gr1553bcbm \
		rtems-gr1553bcbm-exttrig \
		rtems-gr1553bcbm-leon2 \
		rtems-gr1553bcbm-leon2-exttrig \
		ethclient \
		bin/

include ../targets.mk
