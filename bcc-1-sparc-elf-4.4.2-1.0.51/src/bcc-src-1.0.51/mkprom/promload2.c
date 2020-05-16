
typedef struct sectype {
    unsigned int    paddr;
    unsigned int    raddr;
    unsigned int    len;
    unsigned int    comp;
    unsigned char   name[16];
}   tt;

extern struct sectype sections[];

__main() { }

void clean(paddr,len)
double *paddr;
int len;
{
    len >>=3;
    while (len >= 0) {
    	paddr[len] = 0;
	len--;
    }
}

mmov(raddr,paddr,len)
int *raddr, *paddr;
int len;
{
    len >>=2;
    while (len >= 0) {
    	paddr[len] = raddr[len];
	len--;
    }
}

extern volatile unsigned int *_uaddr;

#define	USR	4/4
#define TXA	0/4

void
putsx (s)
     unsigned char *s;
{
    volatile unsigned int *ubase;
    if (((getpsr()>>24) & 0x0f) != 3) ubase = (int *) 0x80000070;
    else ubase = _uaddr;
    while (s[0] != 0)
      {
	  while ((ubase[USR] & 4) == 0);
	  ubase[TXA] = *s;
	  s++;
      }
}

void
puthex (h)
     unsigned int h;
{
  int i = 0;
  char b[9];
  for (i = 0;i < 8;i++,h<<=4) {
    char c = ((h & 0xf0000000) >> 28) & 0xf;
    if (c >= 10) {
      c += 'a' - 10;
    } else {
      c += '0';
    }
    b[i] = c;
  }
  b[8] = 0;
  putsx(b);
}

extern int ramsize, etext, freq, bmsg, _stack;

main()
{
    int   secnum;
    void  (*prog) ();

    prog = (void *) sections[0].paddr;
    secnum = 0;
    while (sections[secnum].paddr) {
	if (sections[secnum].paddr != 0x800) {
	  if (bmsg)
	    {
	      putsx (" moving ");
	      putsx (sections[secnum].name);
	      putsx (" from 0x");
	      puthex(sections[secnum].raddr);
	      putsx (" to 0x");
	      puthex(sections[secnum].paddr);
	      putsx ("\n\r");
	    }
	  
	  mmov(sections[secnum].raddr, 
	       sections[secnum].paddr, sections[secnum].len);
	  
	}
	secnum++;
    }
    prog();
}

