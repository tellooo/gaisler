

/* IOMMU Setup */
#define IOMMU_BUS_CPU 0
#define IOMMU_BUS_MEM 1
#define IOMMU_PREFETCH_DISABLE 0
#define IOMMU_PREFETCH_ENABLE 2

/* MMU Options */
#define MMU_ENABLE 1
#define MMU_DISABLE 0

/* L2C options */
#define L2C_ENABLE 2
#define L2C_DISABLE 0
#define L2C_MTRR_ENABLE 1
#define L2C_MTRR_DISABLE 0


extern void address_region_setup(int mmu_opts, int l2c_opts, int iommu_opts);
