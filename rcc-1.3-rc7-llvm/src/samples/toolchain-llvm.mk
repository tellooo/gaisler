# selects LLVM/Clang compiler
CC=$(CROSS)clang
CPP=$(CROSS)clang++
LD=$(CROSS)ld
RCC_TOOLCHAIN=llvm

TOOL_SPECIFIC_CFLAGS=

# LLVM Clang does not support all BSPs of RCC:
BSPS=leon3 leon3_sf leon3_smp gr712rc gr712rc_smp gr740 gr740_smp

# Setup hardware specific compiler flags based on BSP selected
ifeq ($(strip $(BSP)),erc32)
$(error LLVM/Clang toolchain does not support ERC32 BSP)
endif
ifeq ($(strip $(BSP)),leon2)
$(error LLVM/Clang toolchain does not support LEON2 BSP)
endif
ifeq ($(strip $(BSP)),at697f)
$(error LLVM/Clang toolchain does not support AT697F BSP)
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
	CPUFLAGS=-mcpu=leon3 -msoft-float
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
	CPUFLAGS=-mcpu=gr712rc
endif
ifeq ($(strip $(BSP)),gr712rc_smp)
	CPUFLAGS=-mcpu=gr712rc
endif
ifeq ($(strip $(BSP)),ut699)
$(error LLVM/Clang toolchain does not support UT699)
endif
ifeq ($(strip $(BSP)),ut700)
$(error LLVM/Clang toolchain does not support UT700)
endif
ifeq ($(strip $(BSP)),gr740_smp)
	CPUFLAGS=-mcpu=gr740
endif
ifeq ($(strip $(BSP)),gr740)
	CPUFLAGS=-mcpu=gr740
endif
