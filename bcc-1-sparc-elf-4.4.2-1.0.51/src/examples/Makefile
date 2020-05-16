MULTIDO    = $(MAKE)
MULTICLEAN = $(MAKE)
MKPROM  = sparc-elf-mkprom
CCOPT   = -g -O3
#-DDP
CC      = sparc-elf-gcc $(CFLAGS) $(CCOPT)
CXX     = sparc-elf-gcc $(CFLAGS) $(CCOPT)
LD      = sparc-elf-gcc 
OBJDUMP = sparc-elf-objdump
PT_OBJS =
#pthread3 pthread4 pthread5 pthread6
LDFLAGS = #-qsvt
GCC-VERSION=$(shell sparc-elf-gcc --version | awk '{ print $$3; exit(0); }')
-include ../Makefile.release
ifeq ($(RELEASE_VERSION),)
RELEASE_VERSION=1.0.0
endif

M_ISMINGW  = $(if $(strip $(shell uname | grep MINGW32)),y,)
PREFIX = $(if $(M_ISMINGW),c:/opt,/opt)/sparc-elf-$(if $(VERSION),$(VERSION),$(shell if which sparc-elf-gcc &> /dev/null; then sparc-elf-gcc --version | awk '{ print $$3; exit(0); }'; fi))
BIN    = $(PREFIX)/bin
CONFIG_PATH = $(PATH)
EXPORT_PATH = export PATH=$(BIN):$(CONFIG_PATH)

TARGETSIM=tsim-leon3 
VFLAG= -v -m -1 -k tsim-leon3

EXTRADIR=

ifeq ($(DESTDIR),)
DESTDIR = .
endif

FLAGS_TO_PASS = \
	"CC=$(CC)" \
	"CFLAGS=$(CFLAGS)" \
	"AR=$(AR)" \
	"RANLIB=$(RANLIB)" \
	"AR_FLAGS=$(AR_FLAGS)" \
	"MAKEINFO=$(MAKEINFO)" \
	"AS=$(AS)" \
	"LD=$(LD)" \
	"CROSS_CFLAGS=$(CROSS_CFLAGS)" \
	"TARGET_CFLAGS=$(TARGET_CFLAGS)" \
	"exec_prefix=$(exec_prefix)" \
	"prefix=$(prefix)" \
	"tooldir=$(tooldir)" \
	"INSTALL=$(INSTALL)" \
	"INSTALL_PROGRAM=$(INSTALL_PROGRAM)" \
	"INSTALL_DATA=$(INSTALL_DATA)" \
	"DESTDIR=$(DESTDIR)"


all:
	$(EXPORT_PATH);$(MULTIDO) $(FLAGS_TO_PASS) multi-do DO="clean build"
	$(EXPORT_PATH);make $(FLAGS_TO_PASS) DESTDIR="." clean build

test:
	$(EXPORT_PATH);$(MULTIDO) $(FLAGS_TO_PASS) multi-do DO="dotest"
	$(EXPORT_PATH);make $(FLAGS_TO_PASS) DESTDIR="." dotest

test-summary:
	@$(EXPORT_PATH);$(MULTIDO) $(FLAGS_TO_PASS) --no-print-directory multi-do DO="dotest-summary"
	@$(EXPORT_PATH);make $(FLAGS_TO_PASS) --no-print-directory DESTDIR="." dotest-summary

build:  $(if $(TESTS),$(TESTS),dhry.exe whetstone.exe hello.exe singlemalloc.exe stanford.exe paranoia.exe prime.exe )
#pthread3.exe pthread4.exe pthread5.exe pthread6.exe pthread7.exe pthread8.exe ) 

ifeq ($(TESTS),)
TESTS=dhry.exe hello.exe stanford.exe whetstone.exe
endif

dotest:
	for i in $(TESTS); do \
	 sh runtest.sh $(VFLAG) -x "$(TARGETSIM)" -o "$(BASIC_1)" $(RELEASE_VERSION)-$(GCC-VERSION)$(EXTRADIR)/$(DESTDIR)/$$i; \
	done; \

dotest-summary:
	@for i in $(TESTS:.exe=); do \
	  echo -n $(RELEASE_VERSION)-$(GCC-VERSION)$(EXTRADIR)/; perl $$i.pl --name $(DESTDIR) $(RELEASE_VERSION)-$(GCC-VERSION)$(EXTRADIR)/$(DESTDIR)/$$i.exe.info ; \
	done; \

#echo  "perl $$i.pl --name $(DESTDIR) $(RELEASE_VERSION)-$(GCC-VERSION)$(EXTRADIR)/$(DESTDIR)/$$i.exe.info "; \
	  
#testpthread1.exe testpthread2.exe
testbuild:
#testpthread2.exe

LDFLAGS_whetstone.exe = -lm
LDFLAGS_paranoia.exe = -lm -DBATCHMODE
#LDFLAGS_pthread3.exe = -lpthread
#LDFLAGS_pthread4.exe = -lpthread
#LDFLAGS_pthread5.exe = -lpthread
#LDFLAGS_pthread6.exe = -lpthread
#LDFLAGS_pthread7.exe = -lpthread
#LDFLAGS_pthread8.exe = -lpthread
#LDFLAGS_testpthread1.exe = -lpthread
#LDFLAGS_testpthread2.exe = -lpthread
#LDFLAGS_testpthread3.exe = -lpthread
#LDFLAGS_testpthread4.exe = -lpthread
#LDFLAGS_testpthread5.exe = -lpthread
#LDFLAGS_testpthread6.exe = -lpthread
#LDFLAGS_testpthread7.exe = -lpthread
#LDFLAGS_testpthread8.exe = -lpthread
LDFLAGS_prime.exe = -lstdc++

%.exe: %.o
	./mkinstalldirs $(RELEASE_VERSION)-$(GCC-VERSION)$(EXTRADIR)/$(DESTDIR)
	$(LD) $^ $(LDFLAGS) $(LDFLAGS_$(@F)) -o $(RELEASE_VERSION)-$(GCC-VERSION)$(EXTRADIR)/$(DESTDIR)/$(@F) 
#	$(OBJDUMP) -d -S $(DESTDIR)/$(@F) >$(DESTDIR)/$(@F).dis
#	$(MKPROM) $(DESTDIR)/$(@F) -o $(DESTDIR)/$(basename $(@F)).prom

clean:
	-rm -f *.o *.exe

distclean:
	find . | grep .exe$$ | xargs rm -f $$1
	find . | grep .dis$$ | xargs rm -f $$1
	find . | grep ~$$ | xargs rm -f $$1
	rm -f progs.tar.gz
	rm -rf progs

progs.tar.gz:
	./mkinstalldirs progs
	for i in `find . | grep .exe$$`; do \
	 dir=`echo $$i | sed -e 's/^.//' | sed -e 's/^\///'`; \
	 ./mkinstalldirs `dirname progs/$$dir`; \
	 install $$i progs/$$dir; \
	done; \
	tar cvf progs.tar progs
	rm -f progs.tar.gz
	gzip progs.tar
	rm -rf progs

multi-do:
	@  compiler="$(CC)"; \
	  for i in `$${compiler} --print-multi-lib 2>/dev/null`; do \
	    dir=`echo $$i | sed -e 's/;.*$$//'`; \
	    if [ "$${dir}" = "." ]; then \
	      true; \
	    else \
	        flags=`echo $$i | sed -e 's/^[^;]*;//' -e 's/@/ -/g'`; \
		$(MAKE) $(FLAGS_TO_PASS) \
				CFLAGS="$(CFLAGS) $${flags}" \
				FCFLAGS="$(FCFLAGS) $${flags}" \
				FFLAGS="$(FFLAGS) $${flags}" \
				ADAFLAGS="$(ADAFLAGS) $${flags}" \
				prefix="$(prefix)" \
				exec_prefix="$(exec_prefix)" \
				GCJFLAGS="$(GCJFLAGS) $${flags}" \
				CXXFLAGS="$(CXXFLAGS) $${flags}" \
				LIBCFLAGS="$(LIBCFLAGS) $${flags}" \
				LIBCXXFLAGS="$(LIBCXXFLAGS) $${flags}" \
				LDFLAGS="$(LDFLAGS) $${flags}" \
				MULTIFLAGS="$${flags}" \
				DESTDIR="$${dir}" \
				INSTALL="$(INSTALL)" \
				INSTALL_DATA="$(INSTALL_DATA)" \
				INSTALL_PROGRAM="$(INSTALL_PROGRAM)" \
				INSTALL_SCRIPT="$(INSTALL_SCRIPT)" \
				$(DO);  \
	    fi; \
	  done; \


freertos1:
	sparc-elf-gcc -I $(PREFIX)/sparc-elf/include/freertos freertos1.c -lfreertos -o freertos1.exe

freertos2:
	sparc-elf-gcc -I $(PREFIX)/sparc-elf/include/freertos freertos2.c -lfreertos -o freertos2.exe
