all:

include ../../defs.mk
include ../../common.mk

BCC_XCFLAGS =
BCC_XASFLAGS =

# Include list of common sources files for this BSP
BCC_SOURCES = $(BCC_COMMON_SOURCES)

# BSP specific sources
BCC_SOURCES += shared/lowlevel_leon3.S
BCC_SOURCES += $(BCC_INT_IRQMP_SOURCES)
BCC_SOURCES += $(BCC_APBUART_SOURCES)
BCC_SOURCES += $(BCC_GPTIMER_SOURCES)
# BSP overrides timer and console initialization
BCC_SOURCES += bsp/$(BSPNAME)/bsp_sysfreq.c
BCC_SOURCES := $(filter-out shared/timer/timer_gptimer_init.c,$(BCC_SOURCES))
BCC_SOURCES += bsp/$(BSPNAME)/bsp_timer_init.c
BCC_SOURCES := $(filter-out shared/console/con_apbuart_init.c,$(BCC_SOURCES))
BCC_SOURCES += bsp/$(BSPNAME)/bsp_con_init.c

BCC_SOURCES += bsp/$(BSPNAME)/src/pin.c
BCC_SOURCES += bsp/$(BSPNAME)/src/pll.c

ifeq (,$(findstring clang,$(CC)))
BSP_EXCLUDES = shared/set_pil.S
BCC_SOURCES := $(filter-out $(BSP_EXCLUDES),$(BCC_SOURCES))
BCC_SOURCES += shared/set_pil_pwrpsr.S
endif

EXTRA_SOURCES += shared/crt0.S
EXTRA_SOURCES += shared/trap/trap_table_mvt.S
EXTRA_SOURCES += shared/trap/trap_table_svt.S

EXTRA_DATA  = $(COMMON_EXTRA_DATA)
EXTRA_DATA := $(filter-out $(BCC_PATH)/shared/linkcmds,$(EXTRA_DATA))
EXTRA_DATA := $(filter-out $(BCC_PATH)/shared/linkcmds-rom,$(EXTRA_DATA))
EXTRA_DATA += $(BSP_DIR)/linkcmds
EXTRA_DATA += $(BSP_DIR)/linkcmds-rom
EXTRA_DATA += $(BSP_DIR)/linkcmds-extprom
EXTRA_DATA += $(BSP_DIR)/linkcmds-spi0
EXTRA_DATA += $(BSP_DIR)/linkcmds-spi1
EXTRA_DATA += $(BSP_DIR)/linkcmds-ext

include ../../bcc.mk
include ../../targets.mk

