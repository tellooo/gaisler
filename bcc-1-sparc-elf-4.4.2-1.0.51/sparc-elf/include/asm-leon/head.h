#ifndef __LEONBARE_HEAD_H
#define __LEONBARE_HEAD_H

/* This is for hard interrupts from level 1-14, 15 is non-maskable (nmi) and
 * gets handled with another macro.
 */
#define TRAP_ENTRY_INTERRUPT(int_level) \
        mov int_level, %l7; rd %psr, %l0; b leonbare_irq_entry; rd %wim, %l3;

#define TRAP_ENTRY(H) \
        rd %psr, %l0; b H; rd %wim, %l3; nop;


#endif /* __SPARC_HEAD_H */
