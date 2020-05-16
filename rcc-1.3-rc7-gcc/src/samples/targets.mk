
# Create output directory
$(ODIR):
	mkdir -p $@

erc32:
	$(MAKE) BSP=erc32 ODIR=bin/erc32 erc32_build

leon2:
	$(MAKE) BSP=leon2 ODIR=bin/leon2 leon2_build

at697f:
	$(MAKE) BSP=at697f ODIR=bin/at697f at697f_build
	
leon3:
	$(MAKE) BSP=leon3 ODIR=bin/leon3 leon3_build

leon3_smp:
	$(MAKE) BSP=leon3_smp ODIR=bin/leon3_smp leon3_smp_build

leon3_sf:
	$(MAKE) BSP=leon3_sf ODIR=bin/leon3_sf leon3_sf_build

leon3_sf_smp:
	$(MAKE) BSP=leon3_sf_smp ODIR=bin/leon3_sf_smp leon3_sf_smp_build

leon3_flat:
	$(MAKE) BSP=leon3_flat ODIR=bin/leon3_flat leon3_flat_build

leon3_flat_smp:
	$(MAKE) BSP=leon3_flat_smp ODIR=bin/leon3_flat_smp leon3_flat_smp_build

leon3_std:
	$(MAKE) BSP=leon3_std ODIR=bin/leon3_std leon3_std_build

leon3_mp:
	$(MAKE) BSP=leon3_mp ODIR=bin/leon3_mp leon3_mp_build

gr712rc:
	$(MAKE) BSP=gr712rc ODIR=bin/gr712rc gr712rc_build

gr712rc_smp:
	$(MAKE) BSP=gr712rc_smp ODIR=bin/gr712rc_smp gr712rc_smp_build

gr740:
	$(MAKE) BSP=gr740 ODIR=bin/gr740 gr740_build

gr740_smp:
	$(MAKE) BSP=gr740_smp ODIR=bin/gr740_smp gr740_smp_build

ut699:
	$(MAKE) BSP=ut699 ODIR=bin/ut699 ut699_build

# Use same settings as for UT700 BSP
ut699e:
	$(MAKE) BSP=ut700 ODIR=bin/ut700 ut700_build

ut700: 
	$(MAKE) BSP=ut700 ODIR=bin/ut700 ut700_build

# Create PROM images using mkprom2, setup MKPROMFLAGS before use.
%.mkprom: $(ODIR)% | $(ODIR)
	@echo "### NOTE: Make sure the MKPROMFLAGS are set to flags that correspond to your target system."
	-$(MKPROM2) $(MKPROMFLAGS) -o $@ $<
