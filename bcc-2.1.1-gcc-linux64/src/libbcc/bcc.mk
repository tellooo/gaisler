#
# libBCC
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
# OUTDIR           - Build directory path
#
# Optional vaiables:
# ------------------
# BCC_XCFLAGS      - Extra compiler flags
# BCC_XASFLAGS     - Extra assembler flags
# BCC_XARFLAGS     - Extra archiver flags
# BCC_LIBDIR       - Path to where the archive will be located.
#                    If OUTDIR is set it defaults to $(OUTDIR)/bcc/lib.
# BCC_OBJDIR       - Path to where the object files will be located
#                    If OUTDIR is set it defaults to $(OUTDIR)/bcc/obj.
# BCC_SOURCES      - Override source selection
# EXTRA_SOURCES    - Override extra source selection (not archived in LIBBCC)
#
# Exported variabels:
# -------------------
# BCC_PATH         - Path to BCC root directory
# BCC_CLEAN        - Files to be cleaned
#                  
# BCC_INCLUDE      - Inlude flags
# BCC_LIBS         - Linker libraries flags
# BCC_OBJECTS      - All BCC object files
# EXTRA_OBJECTS    - All extra object files
# LIBBCC           - BCC library target
#
###############################################################################

ifeq ($(BCC_PATH),)
BCC_PATH   :=$(shell dirname  $(lastword $(MAKEFILE_LIST)))
endif

BCC_CLEAN   =$(BCC_OBJDIR)/* $(LIBBCC)
BCC_INCLUDE =-I$(BCC_PATH)/shared/include
BCC_LIBS    =-L$(BCC_LIBDIR) -lbcc
BCC_OBJECTS =$(BCC_SOURCES:%=$(BCC_OBJDIR)/%.o)
EXTRA_OBJECTS =$(EXTRA_SOURCES:%=$(BCC_OBJDIR)/%.o)
ALL_OBJECTS = $(BCC_OBJECTS) $(EXTRA_OBJECTS)
LIBBCC      =$(BCC_LIBDIR)/libbcc.a

BCC_SOURCES ?=
EXTRA_SOURCES ?=

_BCC_DEPS  = $(ALL_OBJECTS:%.o=%.d)

ifeq ($(OUTDIR),)
$(error "OUTDIR not set")
#_BCC_OUTDIR=$(BCC_PATH)
else 
_BCC_OUTDIR=$(OUTDIR)/$(BCC_PATH)
endif

ifeq ($(BCC_LIBDIR),)
BCC_LIBDIR=$(_BCC_OUTDIR)/lib
endif
ifeq ($(BCC_OBJDIR),)
BCC_OBJDIR=$(_BCC_OUTDIR)/obj
endif

$(filter %.c.d,$(_BCC_DEPS)): $(BCC_OBJDIR)/%.d: $(BCC_PATH)/%
	@test -d $(@D) || mkdir -p $(@D)
	@set -e; rm -f $@; \
	$(CC) $(CFLAGS) $(BCC_XCFLAGS) $(BCC_INCLUDE) -MM -MT $(@:.d=.o) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(filter %.S.d,$(_BCC_DEPS)): $(BCC_OBJDIR)/%.d: $(BCC_PATH)/%
	@test -d $(@D) || mkdir -p $(@D)
	@set -e; rm -f $@; \
	$(CC) $(ASFLAGS) $(BCC_XASFLAGS) $(BCC_INCLUDE) -MM -MT $(@:.d=.o)  $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

-include $(_BCC_DEPS) 

# Workaround to generate errors for missing sourcefiles
$(addprefix $(BCC_PATH)/,$(BCC_SOURCES)): %:
	$(error File $@ not found)
$(addprefix $(BCC_PATH)/,$(EXTRA_SOURCES)): %:
	$(error File $@ not found)

$(filter %.c.o,$(ALL_OBJECTS)): $(BCC_OBJDIR)/%.o: $(BCC_PATH)/%
	@test -d $(@D) || mkdir -p $(@D)
	$(CC) $(CFLAGS) $(BCC_XCFLAGS) $(BCC_INCLUDE) $(_BCC_OSAL_DEFINE) -o $@ -c $<

$(filter %.S.o,$(ALL_OBJECTS)): $(BCC_OBJDIR)/%.o: $(BCC_PATH)/%
	@test -d $(@D) || mkdir -p $(@D)
	$(AS) $(ASFLAGS) $(BCC_XASFLAGS) $(BCC_INCLUDE) -o $@ -c $<

$(LIBBCC): $(BCC_OBJECTS)
	@test -d $(@D) || mkdir -p $(@D)
	$(AR) $(ARFLAGS) $(BCC_XAFRLAGS) $@ $(filter %.o,$^)

