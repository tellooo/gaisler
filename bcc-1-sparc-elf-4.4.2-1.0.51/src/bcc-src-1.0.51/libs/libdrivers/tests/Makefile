PREFIX = /opt/sparc-elf
LIBDIR=$(PREFIX)/sparc-elf/lib
INCDIR=$(PREFIX)/sparc-elf/include

all:
	sparc-elf-gcc -g -I $(INCDIR)/libdriver test_gpio.c -o test_gpio.exe -ldrivers -llbthread
	sparc-elf-objdump -l -S -d test_gpio.exe > test_gpio.exe.dis
	sparc-elf-gcc -g -I $(INCDIR)/libdriver test_uart.c -o test_uart.exe -ldrivers -llbthread
	sparc-elf-objdump -l -S -d test_uart.exe > test_uart.exe.dis
