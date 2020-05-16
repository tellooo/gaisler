TOOLCHAIN_NAME=sparc-gaisler-rtems5
CROSS=$(TOOLCHAIN_NAME)-

#SAMPLES_ROOT=$(abspath $(dir $(lastword $(MAKEFILE_LIST))))
SAMPLES_ROOT=$(dir $(lastword $(MAKEFILE_LIST)))

# To set custom hardware flags, set BSP="" and then CPUFLAGS="<flag>..." to 
# replace only the hardware flags or CFLAGS="<flag>...> to replace all the
# flags
ifeq ($(strip $(BSP)),)
  # Default to LEON3 BSP if not selected
  BSP=leon3
endif
CPUFLAGS=
BSPFLAGS=-qbsp=$(BSP)
BSPODIR=bin/$(BSP)/
ifeq ($(strip $(BSP)),ut699e)
$(error When building for UT699E use UT700 BSP)
endif

# default toolchain is GCC, set to RCC_TOOLCHAIN=llvm here to select LLVM/Clang
RCC_TOOLCHAIN=llvm
include $(SAMPLES_ROOT)/toolchain-$(RCC_TOOLCHAIN).mk

MKPROM2=mkprom2
# Add Custom BSPs here:

CONFIG_DEPS=$(SAMPLES_ROOT)config*.c

PATH:=$(PATH):$(shell pwd)/../../bin

CFLAGS=-Wall -g -O2 -Werror $(TOOL_SPECIFIC_CFLAGS)

INCLUDE=-I$(SAMPLES_ROOT) 
override CFLAGS+=$(INCLUDE)

ODIR:=
# Make sure the ODIR ends with a /
ifneq ($(strip $(ODIR)),)
ifneq ($(strip $(ODIR)),$(dir $(strip $(ODIR))))
       override ODIR:=$(ODIR)/
endif
endif

# Default AMP configuration
# Shared memory is 0x40000000-0x40001000
MP_SHM_START=0x40000000
MP_SHM_SIZE=0x1000
# Variables MP_TEXT, MP_TEXT, MP_UART should be a list have MP_NODES number of 
# addresses. Each node has 4MB of memory (execpt the first which has 4MB-1k)
MP_NODES=2
MP_TEXT=0x40001000 0x40400000
MP_STACK=0x403fff00 0x407fff00
MP_UART=0x80000100 0x80000600
# Variables MP_MEMC, MP_IRQMP, MP_GPT needs to be set if not located at default
# location, since MP_MKRPROMFLAGS includes the -nopnp flag
MP_MEMC=
MP_IRQMP=
MP_GPT=

# Default MKPROM2 flags
MKPROMFLAGS=-baud 38400 -freq 50 -memcfg1 0x0003c2ff -memcfg2 0x92c46000 -memcfg3 0x001d2000
MP_MKRPROMFLAGS=-mp -mpstack $(MP_NODES) $(MP_STACK) -mpentry $(MP_NODES) $(MP_TEXT) -mpuart $(MP_NODES) $(MP_UART) -nopnp

# Set other options than CPUFLAGS (set by toolchain-gnu/llvm.mk)
ifeq ($(strip $(BSP)),gr712rc)
	MP_UART=0x80000100 0x80100100
        MKPROMFLAGS=-baud 38400 -freq 48 -romwidth 8 -romsize 8192 -ramwidth 32 -ramcs 1 -ramsize 8192
endif
ifeq ($(strip $(BSP)),ut699)
	MKPROMFLAGS=-baud 38400 -freq 66 -romwidth 32 -romsize 16384 -ramwidth 32 -ramcs 1 -ramsize 4096
endif
ifeq ($(strip $(BSP)),gr740)
        MP_MAX_NODES=2
        MP_SHM_START=0x00000000
        MP_SHM_SIZE=0x1000
        MP_TEXT=0x00001000 0x00400000 
        MP_STACK=0x003fff00 0x007fff00
        MP_UART=0xff900000 0xff901000
        MP_MEMC=0xffe00000
	MP_IRQMP=0xff904000
	MP_GPT=0xff908000

        MKPROMFLAGS=-baud 38400 -freq 200 -sparcleon0 -rstaddr 0xc0000000 -romwidth 8 -romsize 8192 -ddrram 2048 -ddrbanks 2 -ddrfreq 400 
endif
# User's custom BSP:
ifeq ($(strip $(BSP)),CUSTOM)
	# Add you custom compiler and BSP flags here
	CPUFLAGS=-mcpu=leon3
	BSPFLAGS=-qbsp=CUSTOM
	BSPOUT=bin/CUSTOM/
endif

TOOLCHAIN_DEP=$(wildcard $(shell pwd)/../../$(TOOLCHAIN_NAME)/$(BSP)/lib/*.a)

# Add target dependent flags
override CFLAGS+=$(CPUFLAGS) $(BSPFLAGS)
ifneq ($(strip $(MP_MEMC)),)
	MP_MKRPROMFLAGS+=-memc $(MP_MEMC)
endif
ifneq ($(strip $(MP_IRQMP)),)
	MP_MKRPROMFLAGS+=-irqmp $(MP_IRQMP)
endif
ifneq ($(strip $(MP_GPT)),)
	MP_MKRPROMFLAGS+=-gpt $(MP_GPT)
endif
