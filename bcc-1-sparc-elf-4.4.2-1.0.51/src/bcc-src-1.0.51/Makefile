include Makefile.release

SVN_GCC_344   = tarfiles/gcc-3.4.4.tar.gz
SVN_GCC_442   = tarfiles/gcc-4.4.2.tar.bz2
SVN_NEWLIB    = tarfiles/newlib-1.13.0.tar.gz
SVN_GDB       = tarfiles/insight-6.4.tar.bz2
SVN_GDB_M     = tarfiles/gdb-6.3-2-mingw.tar.bz2
SVN_BINUTILS2 = tarfiles/binutils-2.16.1.tar.gz
SVN_BINUTILS19 = tarfiles/binutils-2.19.51.tar.bz2
SVN_GMP           = tarfiles/gmp-4.2.4.tar.bz2
SVN_MPFR          = tarfiles/mpfr-2.4.1.tar.bz2


SYSCOPY=$(if $(filter MINGW32%,$(shell uname)),--usecopy,)

BASEHTTP=http://www.gaisler.com/anonftp/bcc/src/tarfiles
download:
	-mkdir $(CURDIR)/tarfiles
	cd $(CURDIR)/tarfiles; \
	wget -N $(BASEHTTP)/insight-6.4.tar.bz2; \
	wget -N $(BASEHTTP)/binutils-2.13.2.1.tar.gz; \
	wget -N $(BASEHTTP)/gmp-4.2.4.tar.bz2; \
	wget -N $(BASEHTTP)/mpfr-2.4.1.tar.bz2; \
	wget -N $(BASEHTTP)/binutils-2.19.51.tar.bz2; \
	wget -N $(BASEHTTP)/flex-2.5.4a-1.exe; \
	wget -N $(BASEHTTP)/gcc-3.2.3.tar.gz; \
	wget -N $(BASEHTTP)/gcc-3.4.4.tar.gz; \
	wget -N $(BASEHTTP)/gcc-core-3.4.4.tar.bz2; \
	wget -N $(BASEHTTP)/gcc-4.4.2.tar.bz2; \
	wget -N $(BASEHTTP)/gdb-6.3-2.tar.bz2; \
	wget -N $(BASEHTTP)/gdb-6.3.tar.gz; \
	wget -N $(BASEHTTP)/newlib-1.13.0.tar.gz; \
	wget -N $(BASEHTTP)/termcap-1.3.1.tar.gz; \
	wget -N $(BASEHTTP)/gdb-7.1.tar.bz2; \
	wget -N $(BASEHTTP)/gdb-6.3-2-mingw.tar.bz2; \


all:
	-mkdir -p $(CURDIR)/toolchains
	-rm -rf /opt/sparc-elf-3.4.4 /opt/sparc-elf-4.4.2 ;
	-rm -rf /opt/sparc-elf-3.4.4-mingw /opt/sparc-elf-4.4.2-mingw ;
	-mkdir -p $(CURDIR)/toolchains/sparc-elf-3.4.4
	-mkdir -p $(CURDIR)/toolchains/sparc-elf-4.4.2
	-mkdir -p $(CURDIR)/toolchains/sparc-elf-3.4.4-mingw
	-mkdir -p $(CURDIR)/toolchains/sparc-elf-4.4.2-mingw
	cd /opt;ln -s $(CURDIR)/toolchains/sparc-elf-3.4.4 sparc-elf-3.4.4
	cd /opt;ln -s $(CURDIR)/toolchains/sparc-elf-4.4.2 sparc-elf-4.4.2
	cd /opt;ln -s $(CURDIR)/toolchains/sparc-elf-3.4.4-mingw sparc-elf-3.4.4-mingw
	cd /opt;ln -s $(CURDIR)/toolchains/sparc-elf-4.4.2-mingw sparc-elf-4.4.2-mingw
	make -f Makefile.gcc-3.4.4 build-all
	make -f Makefile.gcc-4.4.2 build-all




install: 
	-tar xzf $(SVN_GCC_344)	      --keep-old-files
	-tar xjf $(SVN_GCC_442)       --keep-old-files
	-tar xjf $(SVN_BINUTILS19)    --keep-old-files
	-tar xzf $(SVN_NEWLIB)	      --keep-old-files 
	-tar xjf $(SVN_GDB)	      --keep-old-files
	-tar xjf $(SVN_GDB_M)         --keep-old-files
	-tar xvf $(SVN_GMP);
	-tar xvf $(SVN_MPFR);


symlinks:
	rm -rf /opt/sparc-elf-3.2.3 /opt/sparc-elf-3.4.4 /opt/sparc-elf-4.1.2 /opt/sparc-elf-4.4.1 /opt/sparc-aout-3.2.3 ;
	cd /opt;ln -s $(CURDIR)/toolchains/sparc-elf-3.2.3 sparc-elf-3.2.3
	cd /opt;ln -s $(CURDIR)/toolchains/sparc-aout-3.2.3 sparc-aout-3.2.3
	cd /opt;ln -s $(CURDIR)/toolchains/sparc-elf-3.4.4 sparc-elf-3.4.4
	cd /opt;ln -s $(CURDIR)/toolchains/sparc-elf-4.1.2 sparc-elf-4.1.2
	cd /opt;ln -s $(CURDIR)/toolchains/sparc-elf-4.4.1 sparc-elf-4.4.1



cleaner:
	-rm -rf *-build

clear:
	-rm -rf gcc-3.2.3-binutils-2.13.2.1-build gcc-3.2.3-build gcc-3.2.3-gdb-6.3-build gcc-3.2.3-newlib-1.13.0-build
	-rm -rf gcc-3.4.4-binutils-2.13.2.1-build gcc-3.4.4-build gcc-3.4.4-gdb-6.3-build gcc-3.4.4-newlib-1.13.0-build
	-rm -rf gcc-4.1.2-binutils-2.13.2.1-build gcc-4.1.2-build gcc-4.1.2-gdb-6.3-build gcc-4.1.2-newlib-1.13.0-build
	-rm -rf gcc-4.4.1-binutils-2.16.1-build gcc-4.4.1-binutils-2.19.51-build gcc-4.4.1-build gcc-4.4.1-gdb-6.3-build gcc-4.4.1-newlib-1.13.0-build
	-rm -rf gcc-4.4.1-gmp-4.2.4-install gcc-4.4.1-gmp-4.2.4-build gcc-4.4.1-mpfr-2.4.1-build gcc-4.4.1-mpfr-2.4.1-install
	-rm -rf slackware rpm 
	-rm -rf glibc-2.5-build- glibc-2.5-build-soft glibc-2.5-build-soft_v8 glibc-2.5-build-v8
	-rm -rf glibc-2.3.2-build- glibc-2.3.2-build-soft glibc-2.3.2-build-soft_v8 glibc-2.3.2-build-v8
	-rm -rf gcc-3.4.4-sparc-linux-binutils-2.16.1-build gcc-3.4.4-sparc-uclinux-binutils-2.16.1-build gcc-4.1.2-sparc-linux-binutils-2.16.1-build
	-rm -rf gcc-sparc-linux-3.4.4-build gcc-sparc-linux-4.1.2-build gcc-sparc-uclinux-3.4.4-build gcc-threadx-3.4.4--build gcc-threadx-3.4.4-binutils-2.16.1-build gcc-threadx-3.4.4-newlib-1.13.0-build
	-rm -rf newlib-1.13.0-build binutils-2.16.1-build

