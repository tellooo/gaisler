/* LEON SRMMU set up routines for:
 *  - Set up MMU context table
 *  - Set up Level 1 and Level 2 MMU Tables and Linking the tables together
 *  - enable MMU, flush TLB and cache
 *  - installing trap handler
 */

#include <rtems.h>
#include <bsp.h>
#include <rtems/bspIo.h>
#include <stdio.h>
#include <inttypes.h>

struct srmmu_tables {
	unsigned int ctx_table[256];/* Contexts */

	/* Context 0 Tables */
	unsigned int table0_1[256]; /* Level1 0x00000000-0xFFFFFFFF */
	/* Context 1 Tables */
	unsigned int table1_1[256]; /* Level1 unused */

	unsigned int table0_2[64];  /* Level2 0x40000000-0x40ffffff */
	unsigned int table1_2[64];  /* Level2 unused */
};

/* SRMMU table setup */
void srmmu_table_setup(void)
{
	struct srmmu_tables *tabs = (void *)0x40380000;
	unsigned int i;
#define INVALID 0
#define PTD 1
#define PTE 2
#define CACHABLE 0x80
#define US_RWX (3<<2) /* Access permission User and Supervisor: R/W/X */
#define US_RW (1<<2) /* Access permission User and Supervisor: R/W */
#define US_RX (2<<2)
#define US_R (0<<2)

	/* Setup Context Table */
	tabs->ctx_table[0] = ((unsigned int)&tabs->table0_1[0] >> 4) | PTD;
	for (i=1; i<256; i++) {
		tabs->ctx_table[i] = INVALID; /* Context not used */
	}

	/*** Setup Context 0 Access ***/

	/* 0x00000000 - 0x00ffffff: PROM access R/X */
	tabs->table0_1[0] = (0x00000000UL >> 4) | CACHABLE | US_RX | PTE; /* MAP 1:1, Do not allow writes */

	/* 0x01000000-0x3fffffff: INVALID */
	for (i=1; i<64; i++) {
		/* 16MByte blocks */
		tabs->table0_1[i] = INVALID;
	}

	/* 0x40000000-0x40ffffff: see Level2 */
	tabs->table0_1[64] = ((unsigned int)&tabs->table0_2[0] >> 4) | PTD;

	/* 0x41000000-0x7fffffff: INVALID */
	for (i=65; i<128; i++) {
		/* 16MByte blocks */
		tabs->table0_1[i] = INVALID;
	}

	/* 0x80000000-0xffffffff: R/W */
	for (i=128; i<256; i++) {
		tabs->table0_1[i] = ((i * 0x1000000UL) >> 4) | US_RW | PTE;
	}

	/* 0x40000000-0x402fffff: R/W/X */
	tabs->table0_2[0]  = (0x40000000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[1]  = (0x40040000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[2]  = (0x40080000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[3]  = (0x400c0000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[4]  = (0x40100000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[5]  = (0x40140000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[6]  = (0x40180000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[7]  = (0x401c0000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[8]  = (0x40200000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[9]  = (0x40240000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[10] = (0x40280000UL >> 4) | CACHABLE | US_RWX | PTE;
	tabs->table0_2[11] = (0x402c0000UL >> 4) | CACHABLE | US_RWX | PTE;

	/* 0x40300000-0x40340000: R */
	tabs->table0_2[12] = (0x40300000UL >> 4) | CACHABLE | US_R | PTE;
	/* 0x40340000-0x4037ffff: PROM R/W access */
	tabs->table0_2[13] = (0x00000000UL >> 4) | CACHABLE | US_RW | PTE;
	/* 0x40380000-0x403fffff: INVALID */
	tabs->table0_2[14] = INVALID; /* MMU Tables */
	tabs->table0_2[15] = INVALID;

	/* 0x40400000-0x40ffffff */
	for (i=16; i<64; i++) {
		/* 256kByte blocks */
		tabs->table0_2[i] = INVALID;
	}
}

/* Physical page extraction from PTP's and PTE's. */
#define SRMMU_CTX_PMASK    0xfffffff0
#define SRMMU_PTD_PMASK    0xfffffff0
#define SRMMU_PTE_PMASK    0xffffff00

/* SRMMU Register addresses in ASI ASI_M_MMUREGS */
#define SRMMU_CTRL_REG           0x00000000
#define SRMMU_CTXTBL_PTR         0x00000100
#define SRMMU_CTX_REG            0x00000200
#define SRMMU_FAULT_STATUS       0x00000300
#define SRMMU_FAULT_ADDR         0x00000400

#define ASI_M_MMUREGS 0x19	/* READ/Write MMU Registers */
#define ASI_MMUFLUSH 0x18    /* FLUSH TLB */
#define ASI_DFLUSH 0x11    /* Flush D-Cache */

static __inline__ void srmmu_set_ctable_ptr(unsigned long paddr)
{
	paddr = ((paddr >> 4) & SRMMU_CTX_PMASK);
	__asm__ __volatile__("sta %0, [%1] %2\n\t"::"r"(paddr),
			     "r"(SRMMU_CTXTBL_PTR),
			     "i"(ASI_M_MMUREGS):"memory");
}

static __inline__ void srmmu_set_context(int context)
{
	__asm__ __volatile__("sta %0, [%1] %2\n\t"::"r"(context),
			     "r"(SRMMU_CTX_REG), "i"(ASI_M_MMUREGS):"memory");
}

static __inline__ void srmmu_set_mmureg(unsigned long regval)
{
	__asm__ __volatile__("sta %0, [%%g0] %1\n\t"::"r"(regval),
			     "i"(ASI_M_MMUREGS):"memory");
}

void __inline__ leon_flush_cache_all(void)
{
	__asm__ __volatile__(" flush ");
      __asm__ __volatile__("sta %%g0, [%%g0] %0\n\t"::"i"(ASI_DFLUSH):"memory");
}

void __inline__ leon_flush_tlb_all(void)
{
	leon_flush_cache_all();
	__asm__ __volatile__("sta %%g0, [%0] %1\n\t"::"r"(0x400),
			     "i"(ASI_MMUFLUSH):"memory");
}

rtems_isr srmmu_bad_trap(rtems_vector_number trap, CPU_Interrupt_frame *isf)
{
	uint32_t real_trap = SPARC_REAL_TRAP_NUMBER(trap);

	printk("!!!BAD TRAP: 0x%" PRIx32 "\n", real_trap);
	
	if (real_trap == 0x01) {
		/* step over instruction */
		printk("!!!generated from PC 0x%08" PRIx32 ", redirecting to content of reg I0\n", isf->tpc);
		isf->pc = isf->i0;
		isf->npc = isf->i0 + 4;
	}
}

/* Example traps generated by MMU on faulty access */
#define MMU_TRAPS 7
int traps[MMU_TRAPS] = {0x09, 0x29, 0x2c, 0x20, 0x21, 0x3c, 0x01};

/* Setup and enable */
void srmmu_setup(void)
{
	struct srmmu_tables *tabs = (void *)0x40380000;
	rtems_interrupt_level level;
	int i;

	rtems_interrupt_disable(level);

	/* Register TRAP Handler for MMU Traps */
	for (i=0; i<MMU_TRAPS; i++) {
		set_vector(
			(rtems_isr_entry) srmmu_bad_trap,
			SPARC_SYNCHRONOUS_TRAP(traps[i]),
			1);
	}

	/* Set Context Pointer of MMU */
	srmmu_set_ctable_ptr((unsigned long)&tabs->ctx_table[0]);

	/* Set Context Number */
	srmmu_set_context(0);

	/* Invalidate all Cache */
	__asm__ __volatile__("flush\n\t");

	/* flush TLB cache */
	leon_flush_tlb_all();

	srmmu_set_mmureg(0x00000001);

	/* flush TLB cache */
	leon_flush_tlb_all();

	/* Flush cache */
	leon_flush_cache_all();

	rtems_interrupt_enable(level);
}

/* Set up MMU tables and enable MMU */
void mmu_setup(void)
{
	srmmu_table_setup();
	srmmu_setup();
	printf("The MMU has been setup and enabled\n");
}
