include ../config.mk

INCLUDE+=-I.

PROGS=test

all: gr740

gr740_build: $(PROGS)

# build options for different systems

override CONFIG_DEPS=config*.c

clean:
	rm -f *.o core test > /dev/null

test: $(CONFIG_DEPS) test.c
	$(CC) $(CFLAGS) test.c -o test


include ../targets.mk
