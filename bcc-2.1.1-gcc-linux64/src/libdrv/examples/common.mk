CC = sparc-gaisler-elf-gcc
SIZE = sparc-gaisler-elf-size

ROOTDIR := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
DRVDIR := $(realpath $(dir  $(ROOTDIR)))

BINDIR ?= bin
OBJDIR ?= obj
BSP?=leon3

ALL_EX  ?=

_CFLAGS  += -qbsp=$(strip $(BSP))
_CFLAGS  += -Os
_CFLAGS  += -mcpu=leon3
_CFLAGS  += -qnano
_CFLAGS  += -qsvt
_CFLAGS  += -Wall -Wextra -pedantic
_CFLAGS  += -I$(DRVDIR)/dist/include
_CFLAGS  += -I$(ROOTDIR)

override CFLAGS := $(_CFLAGS) $(CFLAGS)

_LDFLAGS ?=
_LDFLAGS += -L$(DRVDIR)/dist/lib
override LDFLAGS := $(_LDFLAGS) $(LDFLAGS)

_LDLIBS  ?=
_LDLIBS  += -ldrv
override LDLIBS := $(_LDLIBS) $(LDLIBS)

