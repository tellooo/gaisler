
void
_exit (int status)
{
   asm("mov 1, %g1; ta 0;\n");
}
