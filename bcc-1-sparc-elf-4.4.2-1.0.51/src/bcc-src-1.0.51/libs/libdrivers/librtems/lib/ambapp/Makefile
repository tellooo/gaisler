### Debug Makefile for just building LIBAMBAPP

CC=/opt/sparc-elf-4.4.1/bin/sparc-elf-gcc
CFLAGS=-Wall -I../../include -g3 -O3 -c

.PHONY: ambapp

ambapp:
	$(CC) $(CFLAGS) ambapp_alloc.c
	$(CC) $(CFLAGS) ambapp.c
	$(CC) $(CFLAGS) ambapp_depth.c
	$(CC) $(CFLAGS) ambapp_freq.c
	$(CC) $(CFLAGS) ambapp_names.c
	$(CC) $(CFLAGS) ambapp_old.c
	$(CC) $(CFLAGS) ambapp_parent.c
	$(CC) $(CFLAGS) ambapp_show.c
