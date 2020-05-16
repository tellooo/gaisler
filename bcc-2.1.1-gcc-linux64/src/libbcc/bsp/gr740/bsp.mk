all:

include ../../defs.mk
include ../../common.mk

BCC_XCFLAGS =
BCC_XASFLAGS =

# Include list of common sources files for this BSP
BCC_SOURCES =  $(BCC_COMMON_SOURCES)

# BSP specific sources
BCC_SOURCES += shared/lowlevel_leon3.S
BCC_SOURCES += $(BCC_INT_IRQMP_SOURCES)
BCC_SOURCES += $(BCC_APBUART_SOURCES)
BCC_SOURCES += $(BCC_GPTIMER_SOURCES)

# Local BSP sources

EXTRA_SOURCES += shared/crt0.S
EXTRA_SOURCES += shared/trap/trap_table_mvt.S
EXTRA_SOURCES += shared/trap/trap_table_svt.S

EXTRA_DATA  = $(COMMON_EXTRA_DATA)

include ../../bcc.mk
include ../../targets.mk

