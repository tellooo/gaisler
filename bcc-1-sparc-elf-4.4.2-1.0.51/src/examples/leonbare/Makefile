MKPROM  = sparc-elf-mkprom
CC      = sparc-elf-gcc $(CFLAGS)
CXX     = sparc-elf-gcc $(CFLAGS)
LD      = sparc-elf-gcc 
OBJDUMP = sparc-elf-objdump
PROG    = thread
CCOPT   = -g -O3 #-qsvt
LDFLAGS = #-qsvt

%.exe: %.o
	mkinstalldirs $(DESTDIR)
	$(LD) -llbthread $^ $(LDFLAGS) $(LDFLAGS_$(@F)) -o $(DESTDIR)/$(@F) 
	$(OBJDUMP) -d -S $(DESTDIR)/$(@F) >$(DESTDIR)/$(@F).dis
	$(MKPROM) $(DESTDIR)/$(@F) -o $(DESTDIR)/$(basename $(@F)).prom

clean:
	-rm -f *.o *.exe $(PROG)

distclean:
	find . | grep .exe$$ | xargs rm -f $$1
	find . | grep .dis$$ | xargs rm -f $$1
	find . | grep ~$$ | xargs rm -f $$1
	rm -f progs.tar.gz
	rm -rf progs
