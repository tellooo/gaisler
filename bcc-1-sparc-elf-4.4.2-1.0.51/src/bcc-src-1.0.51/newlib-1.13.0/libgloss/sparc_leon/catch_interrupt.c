/*

    LEON2/3 LIBIO low-level routines 
    Written by Jiri Gaisler.
    Copyright (C) 2004  Gaisler Research AB

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include <asm-leon/leonstack.h>
#include <asm-leon/irq.h>
#define NULL 0
struct irqaction *_irqtbl[32] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
struct irqaction _oirqtbl[32] = { INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,
				  INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,
				  INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,
				  INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,
				  INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,
				  INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,
				  INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,
				  INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION,INIT_IRQACTION };

int catch_interrupt (int func, int irq) {
  struct irqaction *a = _irqtbl[irq];
  struct irqaction *n = &_oirqtbl[irq];
  if (irq >= 32)
      return 0;
  
  while (a) {
    if (a == n) {
      int tmp = (int) a->handler;
      a->handler = (irqhandler)func;
      return tmp;
    }
    a = a ->next;
  }
  n->handler = (irqhandler)func;
  chained_catch_interrupt(irq,n);
  return 0;
}

void chained_catch_interrupt (int irq, struct irqaction *a ) {
  a ->next = _irqtbl[irq];
  _irqtbl[irq] = a;
}

int no_inirq_check = 0;
int inirq[32] = { 0,0,0,0,0,0,0,0,
		  0,0,0,0,0,0,0,0,
		  0,0,0,0,0,0,0,0,
		  0,0,0,0,0,0,0,0 };
extern struct irqmp_type irqmp;

void (*handler_irq_pre)(void) = 0;
void (*handler_irq_post)(void) = 0;
handler_irq (int irq, struct leonbare_pt_regs *pt_regs) { 
  struct irqaction *a;
  
  if (irq == irqmp.eirq) irq = irqmp.addr[48] & 0x1f;
  if (!irq) irq = irqmp.eirq;

  a = _irqtbl[irq];

  while (a) {
    if (a->handler) {
#ifndef CONFIG_LEONBARE_NONESTEDIRQ
	if (no_inirq_check || !(inirq[irq])) {
#endif      
	inirq[irq] ++;
	if (handler_irq_pre)
		handler_irq_pre();	
	a->handler(irq, a->dev_id, pt_regs);
	if (handler_irq_post)
		handler_irq_post();	
	inirq[irq] --;
#ifndef CONFIG_LEONBARE_NONESTEDIRQ
      }
#endif      
    }
    a = a->next;
  }
}

schedulehandler schedule_callback = 0;

static unsigned int genseq[4] = {
    0xae102000, /* mov  0, %l7 */
    0x27100049, /* sethi  %hi(0x40012400), %l3 */
    0x81c4e2f4, /* jmp  %l3 + 0x2f4	! 400126f4 <leonbare_irq_entry> */
    0xa1480000  /* rd  %psr, %l0 */
};

void traptable_genjmp(unsigned long *p, int i, int arg, unsigned int fn) {
    p[i*4+0] =  genseq[0] | arg;
    p[i*4+1] = (genseq[1] & ~(((unsigned long)-1) >> 10)) | (fn >> 10);
    p[i*4+2] = (genseq[2] &  (((unsigned long)-1) << 13)) | (fn & 0x3ff);
    p[i*4+3] =  genseq[3];
}

unsigned long sparc_leon23_get_tbr_base(void) {
	return sparc_leon23_get_tbr() & ~(0x1000-1); 
}
