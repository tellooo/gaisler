include ../config.mk

INCLUDE+=-I.

PROGS=test

all: leon3 gr740


PROGS_ONAME=$(addprefix $(ODIR),$(PROGS))

leon3_build: $(PROGS_ONAME)
gr740_build: $(PROGS_ONAME)

# build options for different systems

override CONFIG_DEPS=config*.c

clean:
	rm -rf bin *.o core test > /dev/null

$(ODIR)test: $(CONFIG_DEPS) test.c | $(ODIR)
	$(CC) $(CFLAGS) test.c -o $@

include ../targets.mk
