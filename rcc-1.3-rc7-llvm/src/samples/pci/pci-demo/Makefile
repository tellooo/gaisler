include ../../config.mk

INCLUDE+=-I.. -I.

PROGS=test

all: $(PROGS)

# build options for different systems

override CONFIG_DEPS=config*.c

clean:
	rm -f *.o core test test-rasta test-rasta-host test-rasta-target > /dev/null

mmu_setup.o: mmu_setup.c
	$(CC) $(CFLAGS) -c $< -o $@

test: $(CONFIG_DEPS) test.c mmu_setup.o
	$(CC) $(CFLAGS) -DHOST_NGMP -DTARGET_NGMP test.c -o test mmu_setup.o

test-rasta-host: $(CONFIG_DEPS) test.c
	$(CC) $(CFLAGS) -DTARGET_NGMP test.c -o test-rasta-host

test-rasta-target: $(CONFIG_DEPS) test.c mmu_setup.o
	$(CC) $(CFLAGS) -DHOST_NGMP test.c -o test-rasta-target mmu_setup.o

test-rasta: $(CONFIG_DEPS) test.c
	$(CC) $(CFLAGS) test.c -o test-rasta

include ../../targets.mk
