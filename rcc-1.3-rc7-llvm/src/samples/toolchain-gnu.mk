# selects GCC compiler
CC=$(CROSS)gcc
CPP=$(CROSS)c++
LD=$(CROSS)ld
RCC_TOOLCHAIN=gnu

TOOL_SPECIFIC_CFLAGS=

# GNU GCC support all BSPs of RCC (LEON2/ERC32 BSPs disabled for now):
BSPS=leon3 leon3_sf leon3_smp gr712rc gr712rc_smp gr740 gr740_smp ut699 ut700

# Temporarily disabled BSPs
#BSPS+=erc32 leon2 at697f leon3_sf_smp leon3_std leon3_mp

# Setup hardware specific compiler flags based on BSP selected
ifeq ($(strip $(BSP)),erc32)
	CPUFLAGS=-tsc691
endif
ifeq ($(strip $(BSP)),leon2)
	CPUFLAGS=-mcpu=leon
endif
ifeq ($(strip $(BSP)),at697f)
	CPUFLAGS=-mcpu=leon -mfix-at697f
endif
ifeq ($(strip $(BSP)),leon3)
	CPUFLAGS=-mcpu=leon3
endif
ifeq ($(strip $(BSP)),leon3_smp)
	CPUFLAGS=-mcpu=leon3
endif
ifeq ($(strip $(BSP)),leon3_sf)
	CPUFLAGS=-mcpu=leon3 -msoft-float
endif
ifeq ($(strip $(BSP)),leon3_sf_smp)
	CPUFLAGS=-mcpu=leon3  -msoft-float
endif
ifeq ($(strip $(BSP)),leon3_flat)
	CPUFLAGS=-mcpu=leon3 -mflat
endif
ifeq ($(strip $(BSP)),leon3_flat_smp)
	CPUFLAGS=-mcpu=leon3 -mflat
endif
ifeq ($(strip $(BSP)),leon3_mp)
	CPUFLAGS=-mcpu=leon3
endif
ifeq ($(strip $(BSP)),leon3_std)
	CPUFLAGS=-mcpu=leon3
endif
ifeq ($(strip $(BSP)),gr712rc)
	CPUFLAGS=-mcpu=leon3 -mfix-gr712rc
endif
ifeq ($(strip $(BSP)),gr712rc_smp)
	CPUFLAGS=-mcpu=leon3 -mfix-gr712rc
endif
ifeq ($(strip $(BSP)),ut699)
	CPUFLAGS=-mcpu=leon -mfix-ut699
endif
ifeq ($(strip $(BSP)),ut700)
	CPUFLAGS=-mcpu=leon3 -mfix-ut700
endif
ifeq ($(strip $(BSP)),gr740_smp)
	CPUFLAGS=-mcpu=leon3
endif
ifeq ($(strip $(BSP)),gr740)
	CPUFLAGS=-mcpu=leon3
endif
