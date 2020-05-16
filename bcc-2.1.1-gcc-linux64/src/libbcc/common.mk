BCC_COMMON_SOURCES = \
        shared/trap/trap_table_svt_tables.S \
        shared/trap/trap_table_svt_level0.S \
        shared/trap/reset_trap_mvt.S \
        shared/trap/reset_trap_svt.S \
        shared/trap/window_trap.S \
        shared/trap/interrupt_trap.S \
        shared/trap/interrupt_trap_flat.S \
        shared/trap/flush_windows_trap.S \
        shared/trap/sw_trap_set_pil.S \
        shared/trap.c \
        shared/get_trapbase.c \
        shared/isr.c \
        shared/isr_register_node.c \
        shared/isr_unregister_node.c \
        shared/isr_register.c \
        shared/isr_unregister.c \
        shared/int_nest.c \
        shared/int_disable_nesting.c \
        shared/int_enable_nesting.c \
        shared/int_set_nesting.c \
        shared/int_nestcount.c \
        shared/fpu.S \
        shared/init.S \
        shared/leon_info.S \
        shared/dwzero.S \
        shared/copy_data.c \
        shared/ambapp.c \
        shared/ambapp_findfirst_fn.c \
        shared/lowlevel.S \
        shared/set_pil.S \
        shared/_exit.S \
        shared/read.c \
        shared/sbrk.c \
        shared/heap.c \
        shared/times.c \
        shared/gettimeofday.c \
        shared/write.c \
        shared/stubs/stubs.S \
        shared/stubs/environ.c

BCC_COMMON_SOURCES += shared/argv.c

BCC_INT_IRQMP_SOURCES  = shared/interrupt/int_irqmp_handle.c
BCC_INT_IRQMP_SOURCES += shared/interrupt/int_irqmp.c
BCC_INT_IRQMP_SOURCES += shared/interrupt/int_irqmp_get_source.c
BCC_INT_IRQMP_SOURCES += shared/interrupt/int_irqmp_mp.c
BCC_INT_IRQMP_SOURCES += shared/interrupt/int_irqmp_map.c
BCC_INT_IRQMP_SOURCES += shared/interrupt/int_irqmp_init.c
BCC_INT_IRQMP_SOURCES += shared/interrupt/int_irqmp_timestamp.c

BCC_INT_LEON2_SOURCES  = shared/interrupt/int_leon2_handle.c
BCC_INT_LEON2_SOURCES += shared/interrupt/int_leon2.c
BCC_INT_LEON2_SOURCES += shared/interrupt/int_leon2_get_source.c
BCC_INT_LEON2_SOURCES += shared/interrupt/int_leon2_mp.c
BCC_INT_LEON2_SOURCES += shared/interrupt/int_leon2_map.c
BCC_INT_LEON2_SOURCES += shared/interrupt/int_leon2_init.c
BCC_INT_LEON2_SOURCES += shared/interrupt/int_leon2_timestamp.c

BCC_APBUART_SOURCES  = shared/console/con_handle.c
BCC_APBUART_SOURCES += shared/console/con_apbuart.c
BCC_APBUART_SOURCES += shared/console/con_apbuart_init.c

BCC_GPTIMER_SOURCES  = shared/timer/timer_handle.c
BCC_GPTIMER_SOURCES += shared/timer/timer_custom.c
BCC_GPTIMER_SOURCES += shared/timer/timer_gptimer.c
BCC_GPTIMER_SOURCES += shared/timer/timer_gptimer_tick.c
BCC_GPTIMER_SOURCES += shared/timer/timer_gptimer_init.c

BCC_LEON2TIMER_SOURCES  = shared/timer/timer_handle.c
BCC_LEON2TIMER_SOURCES += shared/timer/timer_custom.c
BCC_LEON2TIMER_SOURCES += shared/timer/timer_leon2.c
BCC_LEON2TIMER_SOURCES += shared/timer/timer_leon2_tick.c

COMMON_EXTRA_DATA  = $(BSP_DIR)/linkcmds.memory
COMMON_EXTRA_DATA += $(BCC_PATH)/shared/linkcmds.base
COMMON_EXTRA_DATA += $(BCC_PATH)/shared/linkcmds
COMMON_EXTRA_DATA += $(BCC_PATH)/shared/linkcmds-rom
COMMON_EXTRA_DATA += $(BCC_PATH)/shared/linkcmds-any

