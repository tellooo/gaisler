#
# libdrv
################################################################################
#
# Required variables:
# -------------------
# CC	           - Compiler
# CFLAGS           - Compiler flags
# AS               - Assembler
# ASFLAGS          - Assembler flags
# AR               - Archiver
# ARFLAGS          - Archiver flags
# SIZE             - Binutils size
# OBJDUMP          - Binutils objdump
#
# Optional variables:
# ------------------
# OUTDIR           - Build directory path
# DRV_XCFLAGS      - Extra compiler flags
# DRV_XASFLAGS     - Extra assembler flags
# DRV_XARFLAGS     - Extra archiver flags
# DRV_LIBDIR       - Path to where the archive will be located.
#                    If OUTDIR is set it defaults to $(OUTDIR)/drv/lib.
# DRV_OBJDIR       - Path to where the object files will be located
#                    If OUTDIR is set it defaults to $(OUTDIR)/drv/obj.
#
# Exported variables:
# -------------------
# DRV_PATH         - Path to DRV root directory
# DRV_CLEAN        - Files to be cleaned
#                  
# DRV_INCLUDE      - Inlude flags
# DRV_LIBS         - Linker libraries flags
# DRV_OBJECTS      - All DRV object files
# LIBDRV           - DRV library target
#
###############################################################################

DRV_PATH        := $(shell dirname  $(abspath $(lastword $(MAKEFILE_LIST))))
DRV_CLEAN        = $(DRV_OBJDIR)/* $(LIBDRV)
DRV_INCLUDE      =
DRV_INCLUDE     += -I$(DRV_PATH)/include
DRV_INCLUDE     += -I$(DRV_PATH)/osal/$(DRV_OSAL_BACKEND)/include
DRV_LIBS         = -L$(DRV_LIBDIR) -ldrv
DRV_OBJECTS      = $(_DRV_SOURCES:%=$(DRV_OBJDIR)/%.o)
LIBDRV           = $(DRV_LIBDIR)/libdrv.a

DRV_OSAL_BACKEND?= bcc2

_DRV_SOURCEDIRS =
_DRV_SOURCEDIRS += ahbstat
_DRV_SOURCEDIRS += apbuart
_DRV_SOURCEDIRS += clkgate
_DRV_SOURCEDIRS += gr1553b
_DRV_SOURCEDIRS += gpio
_DRV_SOURCEDIRS += grcan
_DRV_SOURCEDIRS += grspw
_DRV_SOURCEDIRS += i2cmst
_DRV_SOURCEDIRS += misc
_DRV_SOURCEDIRS += memprot
_DRV_SOURCEDIRS += memscrub
_DRV_SOURCEDIRS += spi
_DRV_SOURCEDIRS += timer

_DRV_SOURCEDIRS += osal/$(DRV_OSAL_BACKEND)
_DRV_SOURCEDIRS += cfg/gr716


_DRV_SOURCES=$(foreach dir, $(_DRV_SOURCEDIRS), $(addprefix $(dir)/, $(notdir $(wildcard $(DRV_PATH)/$(dir)/*.c))))
$(info building $(_DRV_SOURCEDIRS))

_DRV_DEPS=$(DRV_OBJECTS:%.o=%.d)
_DRV_DIRECTORIES=$(dir $(DRV_OBJECTS))

ifeq ($(OUTDIR),)
_DRV_OUTDIR=.
else 
_DRV_OUTDIR=$(OUTDIR)
endif

# If the output directories are common and used for more libraries then this,
# then it's up the the parent makefile to declare a target to build it
ifeq ($(DRV_LIBDIR),)
DRV_LIBDIR=$(_DRV_OUTDIR)/lib
_DRV_DIRECTORIES+=$(DRV_LIBDIR)/
endif
ifeq ($(DRV_OBJDIR),)
DRV_OBJDIR=$(_DRV_OUTDIR)/obj
_DRV_DIRECTORIES+=$(DRV_OBJDIR)/
endif

$(LIBDRV): $(DRV_OBJECTS) | $(DRV_LIBDIR)
	@$(AR) $(ARFLAGS) $(DRV_XARFLAGS) $@ $(filter %.o,$^) > /dev/null

# The mkdir below is a workaround. It should not be needed, but the directory
# order-only-prerequisite dosn't want to work properly for subfolders
$(filter %.c.d,$(_DRV_DEPS)): $(DRV_OBJDIR)/%.d: $(DRV_PATH)/% | $(dir $(DRV_OBJDIR)/%.d)
	@mkdir -p $(dir $(DRV_OBJDIR)/$*.d)
	@set -e; rm -f $@; \
	$(CC) $(CFLAGS) $(DRV_XCFLAGS) $(DRV_INCLUDE) -MM -MT $(@:.d=.o) $(_DRV_OSAL_DEFINE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# The mkdir below is a workaround. It should not be needed, but the directory
# order-only-prerequisite dosn't want to work properly for subfolders
$(filter %.S.d,$(_DRV_DEPS)): $(DRV_OBJDIR)/%.d: $(DRV_PATH)/%.S | $(dir $(DRV_OBJDIR)/%.d)
	@mkdir -p $(dir $(DRV_OBJDIR)/$*.d)
	@set -e; rm -f $@; \
	$(CC) $(CFLAGS) $(DRV_XCFLAGS) $(DRV_INCLUDE) -MM -MT $(@:.d=.o) $(_DRV_OSAL_DEFINE) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(_DRV_DEPS) 

$(filter %.c.o,$(DRV_OBJECTS)): $(DRV_OBJDIR)/%.o: $(DRV_PATH)/% | $(dir $(DRV_OBJDIR)/%.o)
	$(CC) $(CFLAGS) $(DRV_XCFLAGS) $(DRV_INCLUDE) $(_DRV_OSAL_DEFINE) -o $@ -c $<

$(filter %.S.o,$(DRV_OBJECTS)): $(DRV_OBJDIR)/%.o: $(DRV_PATH)/% | $(dir $(DRV_OBJDIR)/%.o)
	$(AS) $(ASFLAGS) $(DRV_XASFLAGS) $(DRV_INCLUDE) -o $@ -c $<

$(sort $(_DRV_DIRECTORIES)):
	mkdir -p $@

# Generate errors for missing source files
$(addprefix $(DRV_PATH)/,$(DRV_SOURCES)): %:
	$(error File $@ not found)

$(DRV_LIBDIR):
	mkdir -p $@

.PHONY: drvsize
drvsize: $(LIBDRV)
	$(SIZE) -t $(LIBDRV)

$(LIBDRV).asm: $(LIBDRV)
	$(OBJDUMP) -d $^ > $@

# We better not do 'rm -rf' here...
.PHONY: drvclean
drvclean:
	rm -f $(LIBDRV) $(LIBDRV).asm $(DRV_OBJECTS) $(_DRV_DEPS)

