### Debug Makefile for just building LIBDRVMGR

CC=/opt/sparc-elf-4.4.1/bin/sparc-elf-gcc
CFLAGS=-Wall -I../../include -g3 -O3 -c

drvmgr:
	$(CC) $(CFLAGS) drvmgr_by_id.c
	$(CC) $(CFLAGS) drvmgr_by_name.c
	$(CC) $(CFLAGS) drvmgr.c
	$(CC) $(CFLAGS) drvmgr_drvinf.c
	$(CC) $(CFLAGS) drvmgr_for_each_dev.c
	$(CC) $(CFLAGS) drvmgr_for_each_list_dev.c
	$(CC) $(CFLAGS) drvmgr_func.c
	$(CC) $(CFLAGS) drvmgr_init.c
	$(CC) $(CFLAGS) drvmgr_list.c
	$(CC) $(CFLAGS) drvmgr_lock.c
	$(CC) $(CFLAGS) drvmgr_print.c
	$(CC) $(CFLAGS) drvmgr_res.c
	$(CC) $(CFLAGS) drvmgr_rw.c
	$(CC) $(CFLAGS) drvmgr_translate.c
	$(CC) $(CFLAGS) drvmgr_unregister.c
