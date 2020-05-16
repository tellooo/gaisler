#include <rtems.h>
#include <stdio.h>
#include <stdlib.h>

#include <bsp.h> /* set_vector() */
#include <rtems/bspIo.h>
#include <grlib/l2c.h>
#include <grlib/griommu.h>
#include <leon.h>
#include "mmu_setup.h"

/* SRMMU has alignment requirements on tables that must be
 * fullfilled:
 *   - CTX Table:        align 0x2000, size 0x400
 *   - Level 1 Table:    align 0x0400, size 0x400
 *   - Level 2 Table:    align 0x0100, size 0x100
 *   - Level 3 Table:    align 0x0100, size 0x100
 */
struct srmmu_tables {
	unsigned int ctx_table[256];

	/* Context 0 - Level 1 tables (16MB/entry) */
	unsigned int ctx0_lvl1[256]; /* Level1 0x00000000-0xFFFFFFFF */
};

struct srmmu_tables __attribute__((aligned(0x2000))) mmu_table;

/* SRMMU table setup:
 *
 * 0x00000000-0x00ffffff: CPU RAM : R/W/X cacheable
 * 0x01000000-0x07ffffff: DMA MEM : R/W non-cacheable
 * 0x08000000-0x7fffffff: N/A     : INVALID
 * 0x80000000-0xffffffff: IO etc. : R/W
 */
void srmmu_table_setup(void)
{
	struct srmmu_tables *tabs = &mmu_table;
	unsigned int i;
#define INVALID 0
#define PTD 1
#define PTE 2
#define CACHABLE 0x80
#define US_RWX (3<<2) /* Access permission User and Supervisor: R/W/X */
#define US_RW (1<<2) /* Access permission User and Supervisor: R/W */

	/* Setup Context Table */
	tabs->ctx_table[0] = ((unsigned int)&tabs->ctx0_lvl1[0] >> 4) | PTD;
	for (i=1; i<256; i++)
		tabs->ctx_table[i] = INVALID; /* Context not used */

	/*** Setup Context 0 Access ***/
	tabs->ctx0_lvl1[0] = (0x00000000UL >> 4) | CACHABLE | US_RWX | PTE; /* MAP 1:1, Allow All */
	/* R/W non-cacheable 112MB */
	tabs->ctx0_lvl1[1] = (0x01000000UL >> 4) | US_RW | PTE; /* MAP 1:1, Allow All */
	tabs->ctx0_lvl1[2] = (0x02000000UL >> 4) | US_RW | PTE; /* MAP 1:1, Allow All */
	tabs->ctx0_lvl1[3] = (0x03000000UL >> 4) | US_RW | PTE; /* MAP 1:1, Allow All */
	tabs->ctx0_lvl1[4] = (0x04000000UL >> 4) | US_RW | PTE; /* MAP 1:1, Allow All */
	tabs->ctx0_lvl1[5] = (0x05000000UL >> 4) | US_RW | PTE; /* MAP 1:1, Allow All */
	tabs->ctx0_lvl1[6] = (0x06000000UL >> 4) | US_RW | PTE; /* MAP 1:1, Allow All */
	tabs->ctx0_lvl1[7] = (0x07000000UL >> 4) | US_RW | PTE; /* MAP 1:1, Allow All */
	/* 0x08000000-0x7fffffff: INVALID */
	for (i=8; i<128; i++) {
		/* 16MByte blocks */
		tabs->ctx0_lvl1[i] = INVALID;
	}
	/* 0x80000000-0xffffffff: R/W */
	for (i=128; i<256; i++) {
		tabs->ctx0_lvl1[i] = ((i * 0x1000000UL) >> 4) | US_RW | PTE;
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
}

#define MMU_TRAPS 6
int traps[MMU_TRAPS] = {0x09, 0x29, 0x2c, 0x20, 0x21, 0x3c};

/* Setup and enable */
void srmmu_setup(int options)
{
	struct srmmu_tables *tabs = &mmu_table;
	rtems_interrupt_level level;
	int i;

	if ((options & MMU_ENABLE) == 0) {
		/* NO MMU */
		printf(" MMU: not used - never enabled\n");
		return;
	}

	/*rtems_interrupt_disable(level);*/
	rtems_interrupt_local_disable(level);

	/* Register TRAP Handler for MMU Traps (only on Boot CPU) */
	if (LEON3_Cpu_Index == _LEON3_Get_current_processor()) {
		for (i=0; i<MMU_TRAPS; i++) {
			set_vector(
				(rtems_isr_entry) srmmu_bad_trap,
				SPARC_SYNCHRONOUS_TRAP(traps[i]),
				1);
		}
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

	rtems_interrupt_local_enable(level);

	printf(" MMU: enabled with DMA regions marked not cacheable\n");
}

#include <amba.h>
#include <ambapp.h>

/* Setup Level 2 cache:
 *
 * 0x01000000 - 0x01ffffff : non-cacheable (16MB)
 * 0x02000000 - 0x03ffffff : non-cacheable (32MB)
 * 0x04000000 - 0x07ffffff : non-cacheable (64MB)
 */
void l2cache_setup(int options)
{
	int i;
	int status = l2cache_status();

	if (status < 0){
		printf("Error while checking L2CACHE\n");
		return;
	}

	if (options & L2C_ENABLE){
		/* Check that L2C is enabled */
		if (L2CACHE_ENABLED(status)==0) {
			printf(" ### L2CACHE is not enabled, enabling ...\n");
			if (l2cache_enable(L2CACHE_OPTIONS_FLUSH_INVALIDATE) != L2CACHE_ERR_OK){
				printf("Error while enabling L2CACHE\n");
				return;
			}
		}else{
			printf(" ### L2CACHE enabled.\n");
		}
	}else{
		/* Check that L2C is disabled */
		if (L2CACHE_ENABLED(status)==0) {
			printf(" ### L2CACHE disabled.\n");
		}else{
			printf(" ### L2CACHE is enabled, disabling ...\n");
			if (l2cache_disable(L2CACHE_OPTIONS_FLUSH_INV_WBACK) != L2CACHE_ERR_OK){
				printf("Error while enabling L2CACHE\n");
				return;
			}
		}
		/* Nothing else to do */
		return;
	}

	if ((options & L2C_MTRR_ENABLE) == 0) {
		printf(" L2CACHE MTRR are disabled\n");
		for (i = 0; i < 16; i++){
			l2cache_mtrr_disable(i);
		}
		return;
	}

	/* Set uncached regions */
	l2cache_mtrr_enable(0, 0x01000000, 0xff000000, 
			L2CACHE_OPTIONS_MTRR_ACCESS_UNCACHED, L2CACHE_OPTIONS_FLUSH_INV_WBACK);
	l2cache_mtrr_enable(1, 0x02000000, 0xfe000000, 
			L2CACHE_OPTIONS_MTRR_ACCESS_UNCACHED, L2CACHE_OPTIONS_FLUSH_INV_WBACK);
	l2cache_mtrr_enable(2, 0x03000000, 0xff000000, 
			L2CACHE_OPTIONS_MTRR_ACCESS_UNCACHED, L2CACHE_OPTIONS_FLUSH_INV_WBACK);
	l2cache_mtrr_enable(3, 0x04000000, 0xfc000000, 
			L2CACHE_OPTIONS_MTRR_ACCESS_UNCACHED, L2CACHE_OPTIONS_FLUSH_INV_WBACK);

	for (i = 4; i < 16; i++){
		l2cache_mtrr_disable(i);
	}

	printf(" L2CACHE: defined MTRR regions as uncacheable\n");
}

static void iommu_print_master(int master)
{
	/* Print each master configuration */
	int vendor, device, routing;
	uint32_t masterinfo;
    if (griommu_master_info(master, &masterinfo) != GRIOMMU_ERR_OK){
        return;
    }
    vendor = griommu_get_master_vendor(masterinfo);
    device = griommu_get_master_device(masterinfo);
    routing = griommu_get_master_routing(masterinfo);
    printf("IOMMU master %d: VENDOR=%s(0x%02x), DEVICE=%s(0x%03x), "
            "BS=%s\n",
            master,
            ambapp_vendor_id2str(vendor), vendor,
            ambapp_device_id2str(vendor,device), device,
            (routing == GRIOMMU_OPTIONS_BUS0? "Primary bus" : "Secondary bus")
          );
}

/* Setup IOMMU master bridge:
 * 
 * prefetch  - 0=disable prefetching, 1=enable prefetching
 * bus       - 0=Route traffic over CPU-BUS, 1=Directly to MEM-BUS
 */
void iommu_setup(int options)
{
	int i;
	int spw_bus = (options & IOMMU_SPW_BUS_MEM);
	int pci_bus = (options & IOMMU_PCI_BUS_MEM);
	int prefetch = (options & IOMMU_PREFETCH_ENABLE);
    int pcimaster[2];
    int spwmaster[4];

	printf(" IOMMU ");
	printf(" prefetching %s, spw traffic routed %s, pci traffic routed %s\n",
		prefetch ? "enabled" : "disabled",
		spw_bus ? "directly to MEM-BUS" : "over CPU-BUS",
		pci_bus ? "directly to MEM-BUS" : "over CPU-BUS");

	if (prefetch == IOMMU_PREFETCH_ENABLE){
		/* Enable prefetching */
		griommu_setup(GRIOMMU_OPTIONS_PREFETCH_ENABLE);
	}else{
		/* Disable prefetching */
		griommu_setup(GRIOMMU_OPTIONS_PREFETCH_DISABLE);
	}

    /* Find SPW and PCI masters */
    spwmaster[0] = griommu_master_find(VENDOR_GAISLER, GAISLER_SPW2_DMA, 0);
    spwmaster[1] = griommu_master_find(VENDOR_GAISLER, GAISLER_SPW2_DMA, 1);
    spwmaster[2] = griommu_master_find(VENDOR_GAISLER, GAISLER_SPW2_DMA, 2);
    spwmaster[3] = griommu_master_find(VENDOR_GAISLER, GAISLER_SPW2_DMA, 3);
    pcimaster[0] = griommu_master_find(VENDOR_GAISLER, GAISLER_GRPCI2, 0);
    pcimaster[1] = griommu_master_find(VENDOR_GAISLER, GAISLER_GRPCI2_DMA, 0);

	/* Select memory bus for pci and spw Masters */
	for (i=0; i<4; i++){
        if (spwmaster[i] < 0){
            printf("IOMMU: Wrong master! %d\n", spwmaster[i]);
            continue;
        }
		griommu_master_setup(spwmaster[i], 0, ((spw_bus == IOMMU_SPW_BUS_MEM)? GRIOMMU_OPTIONS_BUS1:GRIOMMU_OPTIONS_BUS0));
        iommu_print_master(spwmaster[i]);
	}
	for (i=0; i<2; i++){
        if (pcimaster[i] < 0){
            printf("IOMMU: Wrong master! %d\n", pcimaster[i]);
            continue;
        }
		griommu_master_setup(pcimaster[i], 0, ((pci_bus == IOMMU_PCI_BUS_MEM)? GRIOMMU_OPTIONS_BUS1:GRIOMMU_OPTIONS_BUS0));
        iommu_print_master(pcimaster[i]);
	}

}

void address_region_setup(int mmu_opts, int l2c_opts, int iommu_opts)
{
	srmmu_table_setup();
	srmmu_setup(mmu_opts);
	l2cache_setup(l2c_opts);
	iommu_setup(iommu_opts);
	printf("\n\n");
}
