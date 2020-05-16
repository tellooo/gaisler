UIPSRC=uip-0.9/uip
VPATH = $(UIPSRC)

CC=sparc-elf-gcc
AR=sparc-elf-ar
RANLIB=sparc-elf-ranlib
AS=sparc-elf-gcc -c
CFLAGS=-O2 -g $(XFLAGS) -I. -I$(UIPSRC)
ASFLAGS=-O2 -g $(XFLAGS)

PREFIX = /opt/sparc-elf
INCDIR=$(PREFIX)/sparc-elf/include
GCCDIR=$(PREFIX)/lib/gcc-lib/sparc-elf/3.2.3
LIBDIR=$(PREFIX)/sparc-elf/lib
DOCDIR=$(PREFIX)/doc
LIBOBJ=libuip.a #liblwip4.a

LUIPOBJ=uip.o uip_open_eth.o uip_greth.o uip_arch.o uip_arp.o

LUIPINC=$(UIPSRC)/uip.h $(UIPSRC)/uip_arp.h uipopt.h uip_open_eth.h uip_greth.h 

multi-all: $(LIBOBJ)

liblwip4.a: 
	make -f Makefile.lwip XFLAGS="$(XFLAGS)" libs

libuip.a: $(LUIPOBJ)
	$(AR) cr libuip.a $(LUIPOBJ)
	$(RANLIB) libuip.a

install:
	install $(LIBOBJ) $(LIBDIR)/$(DESTDIR)

install-headers:
	if [ ! -d $(INCDIR)/uip ] ; then mkdir $(INCDIR)/uip; fi
	install $(LUIPINC) $(INCDIR)/uip

install-dummy:
	make clean all
	if [ ! -d $(INCDIR)/uip ] ; then mkdir $(INCDIR)/uip; fi
	install $(LUIPINC) $(INCDIR)/uip
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

clean distclean:
	-rm -rf *.o *.a

install-src:
	cp -rp * /opt/sparc-elf/src/net


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

