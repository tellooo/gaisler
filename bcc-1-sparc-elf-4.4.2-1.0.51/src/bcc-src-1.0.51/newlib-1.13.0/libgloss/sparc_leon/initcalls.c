#include <asm-leon/leon.h>
#include <asm-leon/elfmacro.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _call_initcalls() {
    
    initcall_t *p = &__leonbare_initcall_start;
    while(p < &__leonbare_initcall_end) {
	if (*p) {
	    (*p)();
	}
	p++;
    }
}

