include ../config.mk

INCLUDE+=-I.

PROGS=test

all: gr740

gr740_build: $(PROGS)

# build options for different systems

override CONFIG_DEPS=config*.c

clean:
	rm -f *.o core test > /dev/null

msgq.o: msgq.c msgq.h
	$(CC) $(CFLAGS) -c $< -o $@

test: $(CONFIG_DEPS) test.c msgq.o
	$(CC) $(CFLAGS) -DMEMSTART=0 -DMEMSIZE=0x08000000 test.c -o test msgq.o

include ../targets.mk
