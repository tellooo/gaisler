#define DREADY 1
#define TREADY 4

extern volatile int *console;

void
outbyte (int c)
{
  volatile int *rxstat;
  volatile int *rxadata;
  int rxmask;
  while ((console[1] & TREADY) == 0);
  console[0] = (0x0ff & c);
  if (c == '\n')
    {
      while ((console[1] & TREADY) == 0);
      console[0] = (int) '\r';
    }
}

int
inbyte (void)
{
  if (!console) return(0);
  while (!(console[1] & DREADY));
  return console[0];
}


