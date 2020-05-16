/************************************************************************/
/*   This file is a part of the mkprom3 boot-prom utility               */
/*   Copyright (C) 2004 Gaisler Research                                */
/*                                                                      */
/*   This library is free software; you can redistribute it and/or      */
/*   modify it under the terms of the GNU General Public                */
/*   License as published by the Free Software Foundation; either       */
/*   version 2 of the License, or (at your option) any later version.   */
/*                                                                      */
/*   See the file COPYING.GPL for the full details of the license.      */
/************************************************************************/

/*
* This file is part of MKPROM.
* 
* MKPROM3, LEON boot-prom utility. 
* Copyright (C) 2004 Gaisler Research - all rights reserved.
* 
*/

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define VAL(x)  strtoul(x,(char **)NULL,0)
#define SECMAX  32
#define SECNAME 16

typedef struct sectype
{
    unsigned int paddr;
    unsigned int raddr;
    unsigned int len;
    unsigned int comp;
    unsigned char name[SECNAME];
}
tt;

struct sectype secarr[SECMAX];
char filename[128] = "a.out";

#ifndef TOOLBASE
#define TOOLBASE "/opt/sparc-elf-3.2.3"
#endif
#ifndef RELEASE_VERSION
#define RELEASE_VERSION "1.0.24"
#endif
//using -DTOOLBASE="\"...\""
//#define TOOLBASE "/opt/sparc-elf"
const char version[] = "v" RELEASE_VERSION;
int secnum = 0;
FILE *dumpfile;

int verbose = 0;
int vverbose = 0;
int leon = 1;
double freq = 5E7;
int comp = 1;
int flash = 0;
int entry[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int starta = 0;
int foffset = 0;
char ofile[128] = "prom.out";

void usage(char *);

int main (argc, argv)
     int argc;
     char **argv;

{

    int n;
    FILE *xfile;
    char buf[1024];
    char cmd[512];
    char msg[128];
    int baud = 19200;
    int dsubaud = 0;
    int i;
    int rstaddr=0x0;
    int mctrl = 0x00880017;
    int memcfg = 0;
    int wrp = 0;
    int ramcs = 1;
    int rambanks = 1;
    int ramsize = 0x200000;
    int romsize = 0x80000;
    int ramws = 0;
    int ramrws = 0;
    int ramwws = 0;
    int ramwidth = 32;
    int romwidth = 0;
    int rmw = 0;
    int romws = 2;
    int romrws = 2;
    int romwws = 2;
    int stack[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int stat = 1;
    int bmsg = 1;
    int bdinit = 0;
    int dump = 0;
    int iows = 7;
    int iowidth = 2;
    int tmp, ioarea, tmp2;
    int sdramsz = 0;
    int nosram = 0;
    int noinit = 0;
    int sdrambanks = 1;
    int sdcas = 0;
    int trp = 20;
    int trfc = 66;
    int colsz = 1;
    double refresh = 7.8;
    double ftmp;
    int refr;
    char flist[512] = "";
    char xlist[512] = "";
    unsigned int startaddr = 0;
    unsigned int entry[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int uaddr[16] = {0x80000100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    unsigned int memcaddr = 0x80000000;
    unsigned int dsuuart_addr = 0x80000700;
    unsigned int gptaddr = 0x80000300;
    int ecos = 0;
    int mp = 0;
    int j = 0;
    int ncpu = 0;
    int uartnr = 0;
    
    printf ("\nLEON MKPROM prom builder for BCC %s\n", version);
    printf ("Copyright Gaisler Research 2004, all rights reserved.\n\n");
	if (argc < 2)
	{
		 usage(argv[0]);
		 exit(0);
	}
    if ((dumpfile = fopen ("xdump.s", "w+")) == NULL)
      {
	  printf ("Failed to open temporary file\n");
	  exit (1);
      }
    while (stat < argc)
      {
	  if (argv[stat][0] == '-')
	    {
		if (strcmp (argv[stat], "-v") == 0)
		  {
		      verbose = 1;
		  }
		else if (strcmp (argv[stat], "-V") == 0)
		  {
		      verbose = 1;
		      vverbose = 1;
		  }
		else if (strcmp (argv[stat], "-ecos") == 0)
		  {
		      ecos = 1;
		  }
		else if (strcmp (argv[stat], "-dsubaud") == 0)
		  {
		      if ((stat + 1) < argc)
			  dsubaud = VAL (argv[++stat]);
		  }
		else if (strcmp (argv[stat], "-rstaddr") == 0)
		  {
		      if ((stat + 1) < argc)
			  startaddr = VAL (argv[++stat]);
		  }
		else if (strcmp (argv[stat], "-baud") == 0)
		  {
		      if ((stat + 1) < argc)
			  baud = VAL (argv[++stat]);
		  }
		else if (strcmp (argv[stat], "-dump") == 0)
		  {
		      dump = 1;
		  }
		else if (strcmp (argv[stat], "-nocomp") == 0)
		  {
		      comp = 0;
		  }
		else if (strcmp (argv[stat], "-wrp") == 0)
		  {
		      wrp = 1;
		  }
		else if (strcmp (argv[stat], "-nomsg") == 0)
		  {
		      bmsg = 0;
		  }
		else if (strcmp (argv[stat], "-bdinit") == 0)
		  {
		      bdinit = 1;
		  }
		else if (strcmp (argv[stat], "-mp") == 0)
		  {
		      mp = 1;
		  }
		else if (strcmp (argv[stat], "-freq") == 0)
		  {
		      if ((stat + 1) < argc)
			  freq = atof (argv[++stat]);
		      freq *= 1E6;
		  }
		else if (strcmp (argv[stat], "-memc") == 0)
		  {
		      if ((stat + 1) < argc)
			  memcaddr = VAL (argv[++stat]);
		  }
		else if (strcmp (argv[stat], "-gpt") == 0)
		  {
		      if ((stat + 1) < argc)
			  gptaddr = VAL (argv[++stat]);
		  }
		else if (strcmp (argv[stat], "-duart") == 0)
		  {
		      if ((stat + 1) < argc)
			  dsuuart_addr = VAL (argv[++stat]);
		  }
		else if (strcmp (argv[stat], "-col") == 0)
		  {
		      if ((stat + 1) < argc)
			  colsz = VAL (argv[++stat]) - 8;
		      if ((colsz < 0) || (colsz > 3))
			  colsz = 1;
		  }
		else if (strcmp (argv[stat], "-start") == 0)
		  {
		      if ((stat + 1) < argc)
			  starta = VAL (argv[++stat]) & ~3;
		  }
		else if (strcmp (argv[stat], "-cas") == 0)
		  {
		      if ((stat + 1) < argc)
			  sdcas = VAL (argv[++stat]) - 2;
		      if ((sdcas < 0) || (sdcas > 1))
			  sdcas = 1;
		  }
		else if (strcmp (argv[stat], "-sdrambanks") == 0)
		  {
		      if ((stat + 1) < argc)
			  sdrambanks = VAL (argv[++stat]);
		      if ((sdrambanks < 1) || (sdrambanks > 4))
			  sdrambanks = 1;
		  }
		else if (strcmp (argv[stat], "-nosram") == 0)
		  {
		      nosram = 1;
		  }
		else if (strcmp (argv[stat], "-noinit") == 0)
		  {
		      noinit = 1;
		  }
		else if (strcmp (argv[stat], "-sdram") == 0)
		  {
		      if ((stat + 1) < argc)
			  sdramsz = VAL (argv[++stat]);
		      sdramsz *= 1024 * 1024;
		  }
		else if (strcmp (argv[stat], "-trfc") == 0)
		  {
		      if ((stat + 1) < argc)
			  trfc = VAL (argv[++stat]);
		  }
		else if (strcmp (argv[stat], "-trp") == 0)
		  {
		      if ((stat + 1) < argc)
			  trp = VAL (argv[++stat]);
		  }
		else if (strcmp (argv[stat], "-refresh") == 0)
		  {
		      if ((stat + 1) < argc)
			  refresh = atof (argv[++stat]);
		  }
		else if (strcmp (argv[stat], "-o") == 0)
		  {
		      strncpy (ofile, argv[++stat], 127);
		      ofile[127] = 0;
		  }
		else if (strcmp (argv[stat], "-ramsize") == 0)
		  {
		      if ((stat + 1) < argc)
			{
			    ramsize = (VAL (argv[++stat])) & 0x03ffff;
			    ramsize *= 1024;
			}
		  }
		else if (strcmp (argv[stat], "-romws") == 0)
		  {
		      if ((stat + 1) < argc)
			{
			    romws = (VAL (argv[++stat])) & 0xf;
			    romrws = romwws = romws;
			}
		  }
		else if (strcmp (argv[stat], "-romsize") == 0)
		  {
		      if ((stat + 1) < argc)
			{
			    romsize = (VAL (argv[++stat])) & 0x01ffff;
			    romsize *= 1024;
			}
		  }
		else if (strcmp (argv[stat], "-romwidth") == 0)
		  {
		      if ((stat + 1) < argc)
			  romwidth = (VAL (argv[++stat]));
		  }
		else if (strcmp (argv[stat], "-romwidth") == 0)
		  {
		      if ((stat + 1) < argc)
			  romwidth = (VAL (argv[++stat]));
		  }
		else if (strcmp (argv[stat], "-iowidth") == 0)
		  {
		      if ((stat + 1) < argc)
			  iowidth = (VAL (argv[++stat]));
		  }
		else if (strcmp (argv[stat], "-ramcs") == 0)
		  {
		      if ((stat + 1) < argc)
			{
			    rambanks = (VAL (argv[++stat])) & 0x0f;
			    if ((rambanks > 0) || (rambanks < 9))
				ramcs = rambanks;
			}
		  }
		else if (strcmp (argv[stat], "-entry") == 0)
		  {
		      if ((stat + 1) < argc)
		      {
			entry[0] = (VAL (argv[++stat])) & ~0x03;
		      }
		  }
		else if (strcmp (argv[stat], "-mpentry") == 0)
		  {
		    i = 0; ncpu = 0;
		    if ((stat + 1) < argc) 
		      ncpu = VAL (argv[++stat]);
		    while (((stat + 1) < argc) && (i < ncpu))
		    {
		      entry[i] = (VAL (argv[++stat])) & ~0x03;
		      i++;
		    }
		  }

		else if (strcmp (argv[stat], "-stack") == 0)
		  {
		      if ((stat + 1) < argc)
		      {
			stack[0] = (VAL (argv[++stat])) & ~0x01f;
		      }
		  }
		else if (strcmp (argv[stat], "-mpstack") == 0)
		  {
		    i = 0; ncpu = 0;
		    if ((stat + 1) < argc) 
		      ncpu = VAL (argv[++stat]);
		    while (((stat + 1) < argc) && (i < ncpu))
		    {
		      stack[i] = (VAL (argv[++stat])) & ~0x01f;
		      i++;
		    }
		  }
		else if (strcmp (argv[stat], "-iows") == 0)
		  {
		      if ((stat + 1) < argc)
			{
			    iows = (VAL (argv[++stat])) & 0xf;
			}
		  }
		else if (strcmp (argv[stat], "-ramws") == 0)
		  {
		      if ((stat + 1) < argc)
			{
			    ramws = (VAL (argv[++stat])) & 0x3;
			    ramrws = ramwws = ramws;
			}
		  }
		else if (strcmp (argv[stat], "-ramrws") == 0)
		  {
		      if ((stat + 1) < argc)
			  ramrws = (VAL (argv[++stat])) & 0x3;
		  }
		else if (strcmp (argv[stat], "-ramwws") == 0)
		  {
		      if ((stat + 1) < argc)
			  ramwws = (VAL (argv[++stat])) & 0x3;
		  }
		else if (strcmp (argv[stat], "-ramwidth") == 0)
		  {
		      if ((stat + 1) < argc)
			  ramwidth = (VAL (argv[++stat]));
		  }
		else if (strcmp (argv[stat], "-rmw") == 0)
		  {
		      rmw = 1;
		  }
		else if (strcmp (argv[stat], "-uart") == 0)
		  {
		      if ((stat + 1) < argc)
			  uaddr[0] = VAL (argv[++stat]);
		  }
		else if (strcmp (argv[stat], "-mpuart") == 0)
		  {
		    i = 0; uartnr = 0;
		    if ((stat + 1) < argc) 
		      uartnr = VAL (argv[++stat]);
		    while (((stat + 1) < argc) && (i < uartnr))
		    {
		      uaddr[i] = VAL (argv[++stat]);
		      i++;
		    }		
		  }
		else
		  {
		    strcat (xlist, " ");
		    strcat (xlist, argv[stat]);
		    strcat (xlist, " ");
		  }
	    }
	  else
	    {
		if (secnum == 0) {
		    strcpy (filename, argv[stat]);
		    strcat (flist, argv[stat]);
		} else {
		    strcat (flist, " ");
		    strcat (flist, argv[stat]);
		    strcat (flist, " ");
		}
		if (!mp) 
		  entry[0] = elf_load (argv[stat]);
		else
		  elf_load (argv[stat]);

		/* 
		else
		{
		  entry[endx] = elf_load (argv[stat]);
		  endx++;
		}
		*/
	    }
	  stat++;
      }
    printf ("creating LEON3 boot prom: %s\n", ofile);
    if (!flash) {
        fprintf (dumpfile, "\n\t.text\n");
        fprintf (dumpfile, "\n\t.global filename\n");
        fprintf (dumpfile, "filename:\n");
        fprintf (dumpfile, "\t.string\t\"%s\"\n", filename);
        fprintf (dumpfile, "\n\t.align 32\n");
        fprintf (dumpfile, "\t.global sections\n");
        fprintf (dumpfile, "sections:\n");
        for (i = 0; i < secnum; i++)
          {
/* 	      if (entry && (i == 0)) */
/* 	          fprintf (dumpfile, "\t.word\t0x%x\n", entry); */
/* 	      else */
	      fprintf (dumpfile, "\t.word\t0x%x\n", secarr[i].paddr); 
	      fprintf (dumpfile, "\t.word\t_section%d\n", i);
	      fprintf (dumpfile, "\t.word\t0x%x\n", secarr[i].len);
	      fprintf (dumpfile, "\t.word\t0x%x\n", secarr[i].comp);
	      fprintf (dumpfile, "\t.string\t\"%s\"\n", secarr[i].name);
	      fprintf (dumpfile, "\n\t.align 32\n");
          }
        fprintf (dumpfile, "\t.word\t0\n");
    }

    fclose(dumpfile);

    if ((dumpfile = fopen ("dump.s", "w+")) == NULL)
      {
	  printf ("Failed to open temporary file\n");
	  exit (1);
      }
    if (leon)
      {
          fprintf (dumpfile, "\n\t.text\n");
	  fprintf (dumpfile,
		   "\n\t.global _memcfg1, _memcfg2, _memcfg3, _memcaddr, _uart, _dsuuart, _scaler,  _uaddr, _gptaddr, _dsuuart_addr \n");
	  if (mp) fprintf (dumpfile,"\n\t.global _uartnr\n");
      }
    fprintf (dumpfile, "\n\t.global ramsize, _stack, _entry\n");
    fprintf (dumpfile, "\t.global freq, configmsg, bmsg, noinit\n");
    fprintf (dumpfile, "freq:\n");
    fprintf (dumpfile, "\t.word\t%d\n", (int) (freq / 1000000));
    fprintf (dumpfile, "bmsg:\n");
    fprintf (dumpfile, "\t.word\t%d\n", bmsg);
    if (leon)
      {
	  switch (iowidth)
	    {
	    case 8: iowidth = 0; break;
	    case 16: iowidth = 1; break;
	    case 32: iowidth = 2;
	    }
	  switch (romwidth)
	    {
	    case 8: romwidth = 0; break;
	    case 16: romwidth = 1; break;
	    case 32: romwidth = 2; break;
	    case 39: romwidth = 3; break;
	    }
	  tmp = romsize;
	  tmp >>= 14;
	  i = 0;
	  while (tmp)
	    {
		i++;
		tmp >>= 1;
	    }
	  tmp = (i << 14) | romrws | (romwws << 4) | (romwidth << 8) | (1 << 19) |
	  	(iows << 20) | (iowidth << 27);
	  fprintf (dumpfile, "_memcfg1:\n");
	  fprintf (dumpfile, "\t.word\t0x%x\n", tmp);
	  tmp = ramsize / ramcs;
	  tmp >>= 14;
	  i = 0;
	  while (tmp)
	    {
		i++;
		tmp >>= 1;
	    }
	  tmp = (i << 9) | ramrws | (ramwws << 2);
	  switch (ramwidth)
	    {
	    case 8: ramwidth = 0; break;
	    case 16: ramwidth = 1; break;
	    case 39: ramwidth = 3; break;
	    default: ramwidth = 2;
	    }
	  tmp |= ramwidth << 4;
	  tmp |= rmw << 6;

	  i = 0;
	  if (sdramsz)
	    {
		tmp2 = sdramsz;
		tmp2 /= (sdrambanks * 8 * 1024 * 1024);
		while (tmp2)
		  {
		      tmp2 >>= 1;
		      i++;
		  }
		tmp |= 0x80184000;
	    }
	  tmp = (tmp & ~(3 << 21)) | (colsz << 21);
	  tmp = (tmp & ~(7 << 23)) | (i << 23);
	  tmp = (tmp & ~(1 << 26)) | (sdcas << 26);
	  if ((2.0E9 / freq) < (double) trp)
	      trp = 1;
	  else
	      trp = 0;
	  ftmp = ((double) trfc) - (3E9 / freq);
	  if (ftmp > 0)
	      trfc = 1 + (ftmp * freq) / 1E9;
	  else
	      trfc = 0;
	  if (trfc > 7)
	      trfc = 7;
	  tmp = (tmp & ~(7 << 27)) | (trfc << 27);
	  tmp = (tmp & ~(1 << 30)) | (trp << 30);
	  refr = (freq * refresh) / 1E6;
	  if (refr > 0x7fff)
	      refr = 0x7fff;
	  if (nosram)
	    {
		ramsize = sdramsz;
		tmp |= 1 << 13;
	    }
	  fprintf (dumpfile, "_memcfg2:\n");
	  fprintf (dumpfile, "\t.word\t0x%x\n", tmp);
	  tmp = (((10 * (long long) freq) / (8 * baud)) - 5) / 10;
	  baud = freq / (8 * (tmp + 1));
	  fprintf (dumpfile, "_uart:\n");
	  fprintf (dumpfile, "\t.word\t0x%08x\n", tmp);
	  fprintf (dumpfile, "_uaddr:\n");
	  if (mp) 
	  {  
	    for (i = 0; i < ncpu; i++)
	      fprintf (dumpfile, "\t.word\t0x%08x\n", uaddr[i]);
	  }
	  else
	    fprintf (dumpfile, "\t.word\t0x%08x\n", uaddr[0]);
	  if (mp) 
	  {
	    fprintf (dumpfile, "_uartnr:\n");
	    fprintf (dumpfile, "\t.word\t %d\n", uartnr);
	  }
	  tmp = 0;
	  if (dsubaud)
	    {
		tmp = (((10 * (int) freq) / (8 * dsubaud)) + 5) / 10 - 1;
		dsubaud = freq / (8 * (tmp + 1));
	    }
	  fprintf (dumpfile, "_dsuuart:\n");
	  fprintf (dumpfile, "\t.word\t%d\n", tmp);
	  /*
	  if (mp)
	  {
	    i = 1; j = 1;
	    while ((i < 16) && (entry[i]))
	    {
	      while (entry[i-1] == entry[j])
		j++;
	      stack[i-1] = (entry[j] - 32) & ~0x01f;	      
	      i++;
	    }
	    if (stackx)
	      stack[i-1] = (stackx - 32) & ~0x01f;
	    else
	      stack[i-1] = 0x40000000 + ramsize - 32;
	  }
	  else
	  {
	  */
	    if (!stack[0])
	      stack[0] = 0x40000000 + ramsize - 32;
	    /* } */
	  tmp = 0;
	  if (sdramsz)
	      tmp = (refr << 12);
	  fprintf (dumpfile, "_memcfg3:\n");
	  fprintf (dumpfile, "\t.word\t0x%x\n", tmp);
	  fprintf (dumpfile, "noinit:\n");
	  fprintf (dumpfile, "\t.word\t%d\n", noinit);
      }

    if (starta)
    {
      entry[0] = starta;
    }
    fprintf (dumpfile, "_entry:\n");
    if (mp) 
    {  
      for (i = 0; i < ncpu; i++)
	fprintf (dumpfile, "\t.word\t0x%x\n", entry[i]);    
    }
    else
      fprintf (dumpfile, "\t.word\t0x%x\n", entry[0]);    
    fprintf (dumpfile, "ramsize:\n");
    fprintf (dumpfile, "\t.word\t0x%x\n", ramsize);
    fprintf (dumpfile, "_stack:\n");
    if (mp) 
    {  
      for (i = 0; i < ncpu; i++)    
	fprintf (dumpfile, "\t.word\t0x%x\n", stack[i]);
    }
    else
      fprintf (dumpfile, "\t.word\t0x%x\n", stack[0]);      
    fprintf (dumpfile, "_memcaddr:\n");
    fprintf (dumpfile, "\t.word\t0x%x\n", memcaddr);
    fprintf (dumpfile, "_gptaddr:\n");
    fprintf (dumpfile, "\t.word\t0x%x\n", gptaddr);
    fprintf (dumpfile, "_dsuuart_addr:\n");
    fprintf (dumpfile, "\t.word\t0x%x\n", dsuuart_addr);

    sprintf (cmd, "  system clock   : %3.1f MHz\\n\\r", freq / 1E6);
    sprintf (msg, "  baud rate      : %d baud\\n\\r", baud);
    strcat (cmd, msg);
    sprintf (msg, "  prom           : %d K, (%d/%d) ws (r/w)\\n\\r",
	     romsize >> 10, romrws, romwws);
    strcat (cmd, msg);
    if (!nosram)
      {
	  sprintf (msg, "  sram           : %d K, %d bank(s),",
		   ramsize >> 10, rambanks);
	  strcat (cmd, msg);
	  sprintf (msg, " %d/", ramrws);
	  strcat (cmd, msg);
	  sprintf (msg, "%d ws (r/w)\\n\\r", ramwws);
	  strcat (cmd, msg);
      }
    else
      {
	  sprintf (msg,
		   "  sdram          : %d M, %d bank(s), %d-bit column\\n\\r",
		   sdramsz >> 10, sdrambanks, colsz + 8);
	  strcat (cmd, msg);
	  sprintf (msg, "  sdram          : ");
	  strcat (cmd, msg);
	  sprintf (msg,
		   "cas: %d, trp: %2.0f ns, trfc: %2.0f ns, refresh %3.1f us\\n\\r",
		   sdcas + 2, (double) (trp + 2) * 1E9 / freq,
		   (double) (trfc + 3) * 1E9 / freq,
		   (double) (refr + 1) * 1E6 / freq);
	  strcat (cmd, msg);
      }

    if (!flash)
      {
	  fprintf (dumpfile, "configmsg:\n");
	  fprintf (dumpfile, "\t.string\t\"%s\"\n\n\t.align 32\n", cmd);
          xfile = fopen ("xdump.s", "rb");
	  if (xfile) {
		fprintf(xfile, "\n\n");
		while (!feof(xfile)) {
			n = fread(buf, 1, 1024, xfile);
			if (n>0) fwrite(buf, n, 1, dumpfile);
		}
    		fclose (xfile);
	  }
      }
    fclose (dumpfile);
    sprintf (cmd,
#if defined(__CYGWIN32__) || defined(__MINGW32__)
	     "%s/bin/sparc-elf-gcc.exe  -qprom%s -O2 -g -N %s -Ttext=0x%x ",
#else
	     "%s/bin/sparc-elf-gcc -qprom%s -O2 -g -N %s -Ttext=0x%x ",
#endif
	     TOOLBASE, mp ? "mp" : "", ecos ? "-Tlinkpromecos" : "-Tlinkprom" , startaddr);
    if (vverbose)
      strcat (cmd, " -v -Wl,-verbose -Wl,-M ");
    if (flash) {
	if (ecos) {
	    strcat (cmd, " -lmkprom2ecos ");
	} else {
	    strcat (cmd, " -lmkprom2 ");
	}
      strcat (cmd, " -lleonbare -qprom2 dump.s ");
      strcat (cmd, flist);
      strcat (cmd, " -e start ");
    }
    else if (mp)
      strcat (cmd, " -lmkprom3mp -lleonbare dump.s ");
    else
      strcat (cmd, " -lmkprom3 -lleonbare dump.s ");
    if (bdinit)
	strcat (cmd, "bdinit.o ");
    if (flash) {
      if (ecos) {
	strcat (cmd, " -qprom2ecos -qnocrtbegin -qnocrtn -lm -o ");
      } else {
	strcat (cmd, " -lm -o ");
      }
    }
    else if (mp)
      strcat (cmd, " -lmkprom3mp -o ");
    else
      strcat (cmd, " -lmkprom3 -o ");
    strcat (cmd, ofile);
    strcat (cmd, xlist);
    if (verbose)
	printf ("\n%s\n", cmd);
    fflush(stdout);
    system (cmd);
    if (!dump)
    {
      fflush(stdout);
#ifdef __MINGW32__
      system("del dump.s");
#else
      system ("rm -f dump.s");
#endif
    }
    if (flash) {
	flash = 2;
	entry[0] = elf_load (ofile);
    }

    exit (0);
}

void usage (char *argv0)
{
	 printf("Usage: %s [options] input_files\n\n", argv0);

	 puts("Mkprom General Options");
	 puts("  -baud <baudrate>\tSet rate of UART A to baudrate. Default value is 19200.");
	 puts("  -bdinit\t\tCall the functions bdinit1() and bdinit2() in file\n\t\t\tbdinit.o during startup. See manual.");
//	 puts("\tThe user can optionally call two user-defined routines, bdinit1() and bdinit2(), during  the boot process. bdinit1() is called after the LEON registers have been initialized but before the memory has been cleared. bdinit2() is called after the memory has been initialized but before the application is loaded. Note that when bdinit1() is called, the stack has not been setup meaning that bdinit1() must be a leaf routine and not allocate any stack space (no local variables). When -bdinit is used, a file called bdinit.o must exist in the current directory, containing the two routines.");
	 puts("  -dump\t\t\tThe intermediate assembly code with the compressed\n\t\t\tapplication and the LEON register values is put in dump.s\n\t\t\t(only for debugging of mkprom).");
	 puts("  -freq <system_clock>\tDefines the system clock in MHz. This value is used to\n\t\t\tcalculate the divider value for the baud rate generator\n\t\t\tand the real-time clock. Default is 50 for LEON.");
	 puts("  -noinit\t\tSuppress all code which initializes on-chip peripherals\n\t\t\tsuch as uarts, timers and memory controllers. This option\n\t\t\trequires -bdinit to add custom initialisation code,\n\t\t\tor the boot process will fail.");
	 puts("  -nomsg\t\tSuppress the boot message.");
	 puts("  -nocomp\t\tDon't compress application. Decreases loading time\n\t\t\ton the expense of rom size.");
	 puts("  -o <outfile>\t\tPut the resulting image in outfile,\n\t\t\trather then prom.out (default).");
	 puts("  -stack <addr>\t\tSets the initial stack pointer to addr.\n\t\t\tIf not specified, the stack starts at top-of-ram.");
	 puts("  -v\t\t\tBe verbose; reports compression statistics\n\t\t\tand compile commands");

	 puts("\nMkprom options for the LEON2 memory controller");
	 puts("  -cas <delay>\t\tSet the SDRAM CAS delay. Allowed values are 2 and 3,\n\t\t\t2 is default.");
	 puts("  -col <bits>\t\tSet the number of SDRAM column bits.\n\t\t\tAllowed values are 8 - 11, 9 is default.");
	 puts("  -nosram\t\tDisables the static RAM and maps SDRAM at\n\t\t\taddress 0x40000000.");
	 puts("  -ramsize <size>\tDefines the total available RAM. Used to initialize\n\t\t\tthe in the memory configuration register(s).\n\t\t\tThe default value is 2048 (2 Mbyte).");
	 puts("  -ramcs <chip_selects>\tSet the number of ram banks to chip_selects.\n\t\t\tDefault is 1.");
	 puts("  -ramws <ws>\t\tSet the number of waitstates during ram reads and writes\n\t\t\tto ws. Default is 0.");
	 puts("  -ramrws <ws>\t\tSet the number of waitstates during ram reads to ws.\n\t\t\tDefault is 0.");
	 puts("  -ramwws <ws>\t\tSet the number of waitstates during ram writes to ws.\n\t\t\tDefault is 0.");
	 puts("  -romws <ws>\t\tSet the number of rom waitstates during read and write\n\t\t\tto ws. Default is 2.");
	 puts("  -romrws <ws>\t\tSet the number of rom waitstates during read to ws.\n\t\t\tDefault is 2.");
	 puts("  -romwws <ws>\t\tSet the number of rom waitstates during write to ws.\n\t\t\tDefault is 2.");
	 puts("  -ramwidth <width>\tSet the data bus width to 8, 16 or 32-bits, default is 32.\n\t\t\tThe prom width is set through the PIO[1:0] ports.");
	 puts("  -rmw\t\t\tPerform read-modify-write cycles during byte\n\t\t\tand halfword writes.");
	 puts("  -sdram <size>\t\tThe amount of attached SDRAM in Mbyte. 0 by default");
	 puts("  -sdrambanks <num_banks> Set the number of populated SDRAM banks.\n\t\t\tDefault is 1.");
	 puts("  -trfc <delay>\t\tSet the SDRAM tRFC parameter (in ns). Default is 66 ns.");
	 puts("  -trp <delay>\t\tSet the SDRAM tRP parameter (in ns). Default is 20 ns.");
	 puts("  -refresh <delay>\tSet the SDRAM refresh period (in us). Default is 7.8 us,\n\t\t\talthough many SDRAMS actually use 15.6 us.");

	 puts("\nMkprom options for LEON3");
	 puts("  -memc <addr>\tSet the address of the memory controller registers.\n\t\tDefault is 0x80000000.");
	 puts("  -gpt <addr>\tSet the address of the timer unit control registers.\n\t\tDefault is 0x80000300.");
	 puts("  -uart <addr>\tSet the address of the UART control registers.\n\t\tDefault is 0x80000100.");

	 puts("\nThe input files must be in aout or elf32 format.\nIf more than one file is specified, all files are loaded by the loader\nand control is transferred to the first segment of the first file.");
}

#define N   4096
#define F   18
#define THRESHOLD  2
#define NIL  N
#define MAGIC_NUMBER '\xaa'
#define EOP '\x55'
#ifndef SEEK_SET
#define SEEK_SET 0
#endif
#ifndef SEEK_CUR
#define SEEK_CUR 1
#endif
#ifndef SEEK_END
#define SEEK_END 2
#endif

unsigned char text_buf[N + F - 1];
int match_position, match_length, lson[N + 1], rson[N + 257], dad[N + 1];
unsigned long textsize = 0, codesize = 0, printcount = 0;
unsigned char CHECKSUM;

typedef struct
{
    char MAGIC;
    unsigned char PARAMS;
    unsigned char CHECKSUM;
    unsigned char dummy;
    unsigned char ENCODED_SIZE[4];
    unsigned char DECODED_SIZE[4];
}
packet_header;

#define PH_SIZE 12

int
PutPacketInfo (buf)
     char *buf;
{
    packet_header PH;

    PH.MAGIC = MAGIC_NUMBER;
    PH.PARAMS = (unsigned char) (((N >> 6) & 0xf0) |
				 ((((F / 18) % 3) << 2) & 0x0c) | (THRESHOLD -
								   1));
    PH.CHECKSUM = CHECKSUM;
    PH.ENCODED_SIZE[0] = (codesize >> 24);
    PH.ENCODED_SIZE[1] = (codesize >> 16);
    PH.ENCODED_SIZE[2] = (codesize >> 8);
    PH.ENCODED_SIZE[3] = codesize;
    PH.DECODED_SIZE[0] = textsize >> 24;
    PH.DECODED_SIZE[1] = textsize >> 16;
    PH.DECODED_SIZE[2] = textsize >> 8;
    PH.DECODED_SIZE[3] = textsize;
    memcpy (buf, &PH, sizeof (packet_header));
    return 0;
}

void
InitTree (void)
{
    int i;

    for (i = N + 1; i <= N + 256; i++)
	rson[i] = NIL;
    for (i = 0; i < N; i++)
	dad[i] = NIL;
}

void
InsertNode (int r)
{
    int i, p, cmp;
    unsigned char *key;

    cmp = 1;
    key = &text_buf[r];
    p = N + 1 + key[0];
    rson[r] = lson[r] = NIL;
    match_length = 0;
    for (;;)
      {
	  if (cmp >= 0)
	    {
		if (rson[p] != NIL)
		    p = rson[p];
		else
		  {
		      rson[p] = r;
		      dad[r] = p;
		      return;
		  }
	    }
	  else
	    {
		if (lson[p] != NIL)
		    p = lson[p];
		else
		  {
		      lson[p] = r;
		      dad[r] = p;
		      return;
		  }
	    }
	  for (i = 1; i < F; i++)
	      if ((cmp = key[i] - text_buf[p + i]) != 0)
		  break;
	  if (i > match_length)
	    {
		match_position = p;
		if ((match_length = i) >= F)
		    break;
	    }
      }
    dad[r] = dad[p];
    lson[r] = lson[p];
    rson[r] = rson[p];
    dad[lson[p]] = r;
    dad[rson[p]] = r;
    if (rson[dad[p]] == p)
	rson[dad[p]] = r;
    else
	lson[dad[p]] = r;
    dad[p] = NIL;
}

void
DeleteNode (int p)
{
    int q;

    if (dad[p] == NIL)
	return;
    if (rson[p] == NIL)
	q = lson[p];
    else if (lson[p] == NIL)
	q = rson[p];
    else
      {
	  q = lson[p];
	  if (rson[q] != NIL)
	    {
		do
		  {
		      q = rson[q];
		  }
		while (rson[q] != NIL);
		rson[dad[q]] = lson[q];
		dad[lson[q]] = dad[q];
		lson[q] = lson[p];
		dad[lson[p]] = q;
	    }
	  rson[q] = rson[p];
	  dad[rson[p]] = q;
      }
    dad[q] = dad[p];
    if (rson[dad[p]] == p)
	rson[dad[p]] = q;
    else
	lson[dad[p]] = q;
    dad[p] = NIL;
}

int
Encode (inbuf, outbuf, buflen, oindex)
     unsigned char *inbuf;
     unsigned char *outbuf;
     int buflen, oindex;
{
    int i, c, len, r, s, last_match_length, code_buf_ptr;
    unsigned char code_buf[17], mask;

    int lindex = 0;

    CHECKSUM = 0xff;
    InitTree ();
    code_buf[0] = 0;
    code_buf_ptr = mask = 1;
    s = 0;
    r = N - F;
    for (i = s; i < r; i++)
	text_buf[i] = ' ';
    for (len = 0; len < F && (lindex < buflen); len++)
      {
	  c = inbuf[lindex++];
	  CHECKSUM ^= c;
	  text_buf[r + len] = c;
      }
    if ((textsize = len) == 0)
	return;
    for (i = 1; i <= F; i++)
	InsertNode (r - i);
    InsertNode (r);
    do
      {
	  if (match_length > len)
	      match_length = len;
	  if (match_length <= THRESHOLD)
	    {
		match_length = 1;
		code_buf[0] |= mask;
		code_buf[code_buf_ptr++] = text_buf[r];
	    }
	  else
	    {
		code_buf[code_buf_ptr++] = (unsigned char) match_position;
		code_buf[code_buf_ptr++] = (unsigned char)
		    (((match_position >> 4) & 0xf0)
		     | (match_length - (THRESHOLD + 1)));
	    }
	  if ((mask <<= 1) == 0)
	    {
		memcpy (&outbuf[oindex], code_buf, code_buf_ptr);
		oindex += code_buf_ptr;
		codesize += code_buf_ptr;
		code_buf[0] = 0;
		code_buf_ptr = mask = 1;
	    }
	  last_match_length = match_length;
	  for (i = 0; i < last_match_length && (lindex < buflen); i++)
	    {
		c = inbuf[lindex++];
		CHECKSUM ^= c;
		DeleteNode (s);
		text_buf[s] = c;
		if (s < F - 1)
		    text_buf[s + N] = c;
		s = (s + 1) & (N - 1);
		r = (r + 1) & (N - 1);
		InsertNode (r);
	    }
	  if ((textsize += i) > printcount)
	    {
		printcount += 1024;
	    }
	  while (i++ < last_match_length)
	    {
		DeleteNode (s);
		s = (s + 1) & (N - 1);
		r = (r + 1) & (N - 1);
		if (--len)
		    InsertNode (r);
	    }
      }
    while (len > 0);
    if (code_buf_ptr > 1)
      {
	  memcpy (&outbuf[oindex], code_buf, code_buf_ptr);
	  oindex += code_buf_ptr;
	  codesize += code_buf_ptr;
      }
    outbuf[oindex++] = EOP;
    if (verbose)
      {
	  printf ("Uncoded stream length: %ld bytes\n", textsize);
	  printf ("Coded stream length: %ld bytes\n", codesize);
	  printf ("Compression Ratio: %.3f\n", (double) textsize / codesize);
      }
}

int
lzss (inbuf, outbuf, len, comp)
     char *inbuf;
     char *outbuf;
     int len;
     int comp;
{
    int index;

    textsize = 0;
    codesize = 0;
    printcount = 0;

    if (comp)
      {
	  index = sizeof (packet_header);
	  Encode (inbuf, outbuf, len, index);
	  if (PutPacketInfo (outbuf))
	    {
		printf ("Error:couldn't write packet header\n");
	    }
      }
    return (codesize);
}

#include <stdarg.h>

dump (section_address, buffer, count)
     int section_address;
     unsigned char *buffer;
     int count;
{
    int i;

    for (i = 0; i < count; i += 4)
      {
	  fprintf (dumpfile, "\t.word\t0x%02x%02x%02x%02x\n",
		   buffer[i], buffer[i + 1], buffer[i + 2], buffer[i + 3]);
      }
}

int
elf_load (fname)
     char *fname;
{
    int cc, c, tmp;
    unsigned char buf[10];
    unsigned long entry;
    char *lzss_buf;
    FILE *xfile;

#ifdef WIN32
    xfile = fopen (fname, "rb");
#else
    xfile = fopen (fname, "r");
#endif

    if (xfile == NULL)
      {
	  printf ("open of %s failed\n", fname);
	  return (-1);
      }
    tmp = ldelf (xfile, dumpfile);
    printf ("\n");
    return (tmp);
}

dumpsec (char *buf, int section_address, int section_size,
	 char *section_name, FILE * dumpfile)
{
    char cmd[512];
    char *lzss_buf;
    if ((secnum == 0) && (section_address == 0))
      {
	printf("Section in rom detected, switching off compression\n");
	  comp = 0;
	  flash = 1;
      }
    if (flash) {
	if (flash == 2) {
	    if (strcmp(".text", section_name) == 0)
	        foffset = section_size;
	    else {
#ifdef __MINGW32__
	      /* Use copy since move complains the file is in use by other process */
	      sprintf(cmd, "copy /y %s tmp.out", ofile);
#else
	      sprintf(cmd, "mv %s tmp.out", ofile);
#endif
	      if (verbose) printf("%s\n", cmd);
	      fflush(stdout);
	      system (cmd);
#if defined(__CYGWIN32__) || defined(__MINGW32__)
	      sprintf(cmd, "%s/bin/sparc-elf-objcopy.exe --change-section-lma %s=0x%x tmp.out %s", 
#else
	      sprintf(cmd, "%s/bin/sparc-elf-objcopy --change-section-lma %s=0x%x tmp.out %s", 
#endif
		TOOLBASE, section_name, foffset + starta, ofile);
	      if (verbose) printf("%s\n", cmd);
	      fflush(stdout);
	      system (cmd);
	      foffset += section_size;
	    }
	}
	secnum++;
        return(0);
    }
    else
	fprintf (dumpfile, "\t .text\n");
    secarr[secnum].paddr = section_address;
    secarr[secnum].len = section_size;
    secarr[secnum].comp = comp;
    strcpy (secarr[secnum].name, section_name);

    fprintf (dumpfile, "\n\t.global _section%1d\n", secnum);
    fprintf (dumpfile, "_section%1d:\n", secnum);

    if (comp)
      {
	  lzss_buf = (char *) malloc (section_size + section_size / 2 + 256);
      }
    secnum++;
    if (comp)
      {
	  section_size = lzss (buf, lzss_buf, section_size, 1);
	  dump (section_address, lzss_buf, section_size + 13);
	  free (buf);
	  free (lzss_buf);
      }
    else
      {
	  dump (section_address, buf, section_size);
	  free (buf);
      }

    return (0);
}

#ifndef WIN32
#include <netinet/in.h>
#endif

#define EI_NIDENT       16

typedef unsigned int Elf32_Addr;
typedef unsigned int Elf32_Word;
typedef unsigned int Elf32_Off;
typedef unsigned short Elf32_Half;

typedef struct
{
    unsigned char e_ident[EI_NIDENT];
    Elf32_Half e_type;
    Elf32_Half e_machine;
    Elf32_Word e_version;
    Elf32_Addr e_entry;
    Elf32_Off e_phoff;
    Elf32_Off e_shoff;
    Elf32_Word e_flags;
    Elf32_Half e_ehsize;
    Elf32_Half e_phentsize;
    Elf32_Half e_phnum;
    Elf32_Half e_shentsize;
    Elf32_Half e_shnum;
    Elf32_Half e_shstrndx;
} Elf32_Ehdr;

#define EI_MAG0	0
#define EI_MAG1	1
#define EI_MAG2	2
#define EI_MAG3	3
#define EM_SPARC 2

typedef struct
{
    Elf32_Word sh_name;
    Elf32_Word sh_type;
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;
    Elf32_Off sh_offset;
    Elf32_Word sh_size;
    Elf32_Word sh_link;
    Elf32_Word sh_info;
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} Elf32_Shdr;

typedef struct
{
    Elf32_Word p_type;
    Elf32_Off p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} Elf32_Phdr;

ldelf (FILE * fp, FILE * dumpfile)
{
    Elf32_Ehdr fh;
    Elf32_Shdr sh, ssh;
    Elf32_Phdr ph;
    char *strtab;
    char *mem;
    unsigned int *memw, i, j;

    fseek (fp, 0, SEEK_SET);
    if (fread (&fh, sizeof (fh), 1, fp) != 1)
      {
	  return (-2);
      }

    if ((fh.e_ident[EI_MAG0] != 0x7f)
	|| (fh.e_ident[EI_MAG1] != 'E')
	|| (fh.e_ident[EI_MAG2] != 'L') || (fh.e_ident[EI_MAG3] != 'F'))
      {
	  return (-2);
      }
    fh.e_machine = ntohs (fh.e_machine);
    if (fh.e_machine != EM_SPARC)
      {
	  printf ("not a SPARC executable (%d)\n", fh.e_machine);
	  return (-2);
      }

    fh.e_entry = ntohl (fh.e_entry);
    fh.e_shoff = ntohl (fh.e_shoff);
    fh.e_phoff = ntohl (fh.e_phoff);
    fh.e_phnum = ntohs (fh.e_phnum);
    fh.e_shnum = ntohs (fh.e_shnum);
    fh.e_phentsize = ntohs (fh.e_phentsize);
    fh.e_shentsize = ntohs (fh.e_shentsize);
    fh.e_shstrndx = ntohs (fh.e_shstrndx);
    fseek (fp, fh.e_shoff + ((fh.e_shstrndx) * fh.e_shentsize), SEEK_SET);
    if (fread (&ssh, sizeof (ssh), 1, fp) != 1)
      {
	  printf ("header: file read error\n");
	  return (-1);
      }
    ssh.sh_name = ntohl (ssh.sh_name);
    ssh.sh_type = ntohl (ssh.sh_type);
    ssh.sh_offset = ntohl (ssh.sh_offset);
    ssh.sh_size = ntohl (ssh.sh_size);
    strtab = (char *) malloc (ssh.sh_size);
    fseek (fp, ssh.sh_offset, SEEK_SET);
    if (fread (strtab, ssh.sh_size, 1, fp) != 1)
      {
	  printf ("string tab: file read error\n");
	  return (-1);
      }

    for (i = 1; i < fh.e_shnum; i++)
      {
	  fseek (fp, fh.e_shoff + (i * fh.e_shentsize), SEEK_SET);
	  if (fread (&sh, sizeof (sh), 1, fp) != 1)
	    {
		printf ("section header: file read error\n");
		return (-1);
	    }
	  sh.sh_name = ntohl (sh.sh_name);
	  sh.sh_addr = ntohl (sh.sh_addr);
	  sh.sh_size = ntohl (sh.sh_size);
	  sh.sh_type = ntohl (sh.sh_type);
	  sh.sh_offset = ntohl (sh.sh_offset);
	  sh.sh_flags = ntohl (sh.sh_flags);
	  if ((sh.sh_type == 1) && (sh.sh_size > 0) && (sh.sh_flags & 2))
	    {
               if (verbose)
		printf ("section: %s at 0x%x, size %d bytes\n",
			&strtab[sh.sh_name], sh.sh_addr, sh.sh_size);
		mem = (char *) malloc (sh.sh_size);
		if (mem != (char *) -1)
		  {
		      if (sh.sh_type == 1)
			{
			    fseek (fp, sh.sh_offset, SEEK_SET);
			    fread (mem, sh.sh_size, 1, fp);
			    memw = (unsigned int *) mem;
			    dumpsec (mem, sh.sh_addr, sh.sh_size,
				     &strtab[sh.sh_name], dumpfile);
			}
		  }
		else
		  {
		      printf ("load address outside physical memory\n");
		      printf ("load aborted\n");
		      return (-1);
		  }
	    }
      }

    free (strtab);
    return (fh.e_entry);
}
