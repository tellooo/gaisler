ifeq ($(BSPNAME),)
$(error "BSPNAME not set")
endif

DEFS_DIR :=$(shell dirname  $(lastword $(MAKEFILE_LIST)))
BSP_DIR := $(DEFS_DIR)/bsp/$(BSPNAME)
ifeq ($(BCC_PATH),)
BCC_PATH :=$(DEFS_DIR)
endif
PREFIX=sparc-gaisler-elf-

# CC is overwritten when using clang/llvm
CC=$(PREFIX)gcc
AS=$(CC)
AR=$(PREFIX)ar

# for the assembler we need to make sure -mrex is not included
# as the assembly files are written in normal sparc
MULTI_FLAGS_AS = $(filter-out -mrex,$(MULTI_FLAGS))

ASFLAGS=-I$(BCC_PATH)/shared/inc $(MULTI_FLAGS_AS)
ifeq (,$(findstring clang,$(CC)))
  # GCC
  ASFLAGS+=-Wa,-Aleon
endif

ASFLAGS+=-g

CFLAGS=-std=c99 -g -O3 -Wall -Wextra -pedantic $(MULTI_FLAGS) -I$(BSP_DIR)/include
CFLAGS+=-fno-builtin

ifeq ($(DESTDIR),)
BCC_DISTDIR=$(DEFS_DIR)/dist
else
BCC_DISTDIR=$(DESTDIR)
endif
$(info BCC_DISTDIR=$(BCC_DISTDIR))

ifeq ($(BUILDDIR),)
OUTDIR=build/$(BSPNAME)/$(MULTI_DIR)
else
OUTDIR=$(BUILDDIR)/$(BSPNAME)/$(MULTI_DIR)
endif
$(info OUTDIR=$(OUTDIR))

BCC_OBJDIR=$(OUTDIR)/obj
BCC_LIBDIR=$(OUTDIR)/lib

INSTALL_DATA = install -m 644

