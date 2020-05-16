all:

include ../../defs.mk
include ../../common.mk

BCC_XCFLAGS =
BCC_XASFLAGS =

# Include list of common sources files for this BSP
BCC_SOURCES =  $(BCC_COMMON_SOURCES)

# BSP specific sources
BCC_SOURCES += shared/lowlevel_leon2.S
BCC_SOURCES += $(BCC_INT_LEON2_SOURCES)
BCC_SOURCES += shared/console/con_handle.c
BCC_SOURCES += bsp/agga4/con_agga4.c
BCC_SOURCES += $(BCC_LEON2TIMER_SOURCES)

# Local BSP sources

EXTRA_SOURCES += shared/crt0.S
EXTRA_SOURCES += shared/trap/trap_table_mvt.S

EXTRA_DATA  = $(COMMON_EXTRA_DATA)

include ../../bcc.mk
include ../../targets.mk

