include ../config.mk

# include config*.c files from samples directory
CFLAGS=-I.. -O0 -g3

all: rtems-mmu

rtems-mmu: rtems-mmu.c mmu_setup.c ../config*.c
	$(CC) $(CFLAGS) -c mmu_setup.c -o mmu_setup.o
	$(CC) $(CFLAGS) rtems-mmu.c mmu_setup.o -o rtems-mmu


clean:
	rm -f *.o rtems-mmu

include ../targets.mk
