MULTIDO    = $(MAKE)
MULTICLEAN = $(MAKE)

CC=sparc-elf-gcc
AS=sparc-elf-gcc -c
XFLAGS =
CFLAGS=-O2 -g $(XFLAGS)
ASFLAGS=-O2 -g $(XFLAGS)

HOSTCC=gcc -DLEON
AR=sparc-elf-ar
RANLIB=sparc-elf-ranlib

#PREFIX=/opt/sparc-elf
PREFIX=/opt/sparc-elf-3.4.4
LIBDIR=$(PREFIX)/sparc-elf/lib
BINDIR=$(PREFIX)/bin
INCDIR=$(PREFIX)/include
DOCDIR=$(PREFIX)/doc
TOOLBASE=$(PREFIX)

#promcore2_qw.o promcore3_qw.o
LIBOBJ= libmkprom3.a libmkprom2.a libmkprom2ecos.a libmkprom3mp.a prominit.o  prominit_svt.o  prominit_mp.o \
	crt0_noatexit.o crt_cpdata.o crt_cpdataecos.o

WIN32-LIBS = $(if $(strip $(shell uname -a | grep -i -E mingw)),-lWs2_32,)
POSTFIX = $(if $(strip $(shell uname -a | grep -i -E mingw)),.exe,)

multi-all:  promload3.o $(LIBOBJ) promload2.o crt2.o
#regwin.o regwin_qw.o

mkprom: mkprom.c
	$(HOSTCC) -DTOOLBASE="\"$(TOOLBASE)\"" -DRELEASE_VERSION="\"$(RELEASE_VERSION)\""  -I$(INCDIR) -g --static mkprom.c -o mkprom -lm $(WIN32-LIBS)

#regwin.o regwin_qw.o
# uncomressed, copy data to ram 
libmkprom2.a:  prompsr.o dummy.o
	$(AR) cr libmkprom2.a  prompsr.o dummy.o #regwin.o regwin_qw.o
	$(RANLIB) libmkprom2.a
# uncomressed, copy data to ram for ecos
libmkprom2ecos.a:  prompsr.o dummy.o
	$(AR) cr libmkprom2ecos.a  prompsr.o dummy.o  #regwin.o regwin_qw.o
	$(RANLIB) libmkprom2ecos.a


# comressed, compress itself to ram
libmkprom3.a:  prompsr.o decomp.o promload3.o  promasm.o  #regwin.o regwin_qw.o
	$(AR) cr libmkprom3.a  prompsr.o decomp.o promload3.o  promasm.o   #regwin.o regwin_qw.o
	$(RANLIB) libmkprom3.a

# MP, comressed, compress itself to ram
libmkprom3mp.a:  prompsr.o decomp.o promload3mp.o  promasm.o  #regwin.o regwin_qw.o
	$(AR) cr libmkprom3mp.a  prompsr.o decomp.o promload3mp.o  promasm.o   #regwin.o regwin_qw.o
	$(RANLIB) libmkprom3mp.a

install:
	install $(LIBOBJ) $(LIBDIR)/$(DESTDIR)

install-mkprom: mkprom
	-cp -p mkprom $(BINDIR)/sparc-elf-mkprom$(POSTFIX)
	install linkprom linkpromecos linkflash $(LIBDIR)

install-mkprom-linkprom: 
	install linkprom linkpromecos linkflash $(LIBDIR)

dummy:
	make clean all
	install $(LIBOBJ) $(LIBDIR)
	make clean all XFLAGS=-mflat
	install $(LIBOBJ) $(LIBDIR)/flat
	make clean all XFLAGS=-msoft-float
	install $(LIBOBJ) $(LIBDIR)/soft
	make clean all XFLAGS="-msoft-float -mflat"
	install $(LIBOBJ) $(LIBDIR)/soft/flat
	make clean all XFLAGS="-msoft-float -mv8"
	install $(LIBOBJ) $(LIBDIR)/soft/v8
	make clean all XFLAGS="-msoft-float -mv8 -mflat"
	install $(LIBOBJ) $(LIBDIR)/soft/v8/flat
	make clean all XFLAGS="-mv8"
	install $(LIBOBJ) $(LIBDIR)/v8
	make clean all XFLAGS="-mv8 -mflat"
	install $(LIBOBJ) $(LIBDIR)/v8/flat
	install linkprom linkflash $(LIBDIR)
	make mkprom
	-cp -p mkprom$(POSTFIX) $(BINDIR)/sparc-elf-mkprom
	cp -p mkprom.html $(DOCDIR)

clean:
	rm -rf *.o *.a mkprom$(POSTFIX) mkprom.exe core *.out dump.s > /dev/null

install-src:
	cp -p * $(BASE)/src/mkprom


multi-do:
	  compiler="$(CC)"; \
	  for i in `$${compiler} --print-multi-lib 2>/dev/null`; do \
	    dir=`echo $$i | sed -e 's/;.*$$//'`; \
	    if [ "$${dir}" = "." ]; then \
	      true; \
	    else \
	        flags=`echo $$i | sed -e 's/^[^;]*;//' -e 's/@/ -/g'`; \
		$(MAKE) $(FLAGS_TO_PASS) \
				CFLAGS="$(CFLAGS) $${flags}" \
				ASFLAGS="$(ASFLAGS) $${flags}" \
				FCFLAGS="$(FCFLAGS) $${flags}" \
				FFLAGS="$(FFLAGS) $${flags}" \
				ADAFLAGS="$(ADAFLAGS) $${flags}" \
				prefix="$(prefix)" \
				PREFIX="$(PREFIX)" \
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


