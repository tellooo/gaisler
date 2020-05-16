
#include <access.h>
#include <pci.h>
#include <pci/access.h>
#include <pci/cfg.h>

int grpci2_cfg_r32(pci_dev_t dev, int ofs, uint32_t *val)
{
	/* Implement 32-bit PCI Configuration Read Access */
	return -1;
}

int grpci2_cfg_r16(pci_dev_t dev, int ofs, uint16_t *val)
{
	uint32_t v;
	int retval;

	if (ofs & 1)
		return PCISTS_EINVAL;

	retval = grpci2_cfg_r32(dev, ofs & ~0x3, &v);
	*val = 0xffff & (v >> (8*(ofs & 0x3)));

	return retval;
}

int grpci2_cfg_r8(pci_dev_t dev, int ofs, uint8_t *val)
{
	uint32_t v;
	int retval;

	retval = grpci2_cfg_r32(dev, ofs & ~0x3, &v);

	*val = 0xff & (v >> (8*(ofs & 3)));

	return retval;
}

int grpci2_cfg_w32(pci_dev_t dev, int ofs, uint32_t val)
{
	/* Implement 32-bit PCI Configuration Write Access */
	return -1;
}

int grpci2_cfg_w16(pci_dev_t dev, int ofs, uint16_t val)
{
	uint32_t v;
	int retval;

	if (ofs & 1)
		return PCISTS_EINVAL;

	retval = grpci2_cfg_r32(dev, ofs & ~0x3, &v);
	if (retval != PCISTS_OK)
		return retval;

	v = (v & ~(0xffff << (8*(ofs&3)))) | ((0xffff&val) << (8*(ofs&3)));

	return grpci2_cfg_w32(dev, ofs & ~0x3, v);
}

int grpci2_cfg_w8(pci_dev_t dev, int ofs, uint8_t val)
{
	uint32_t v;
	int retval;

	retval = grpci2_cfg_r32(dev, ofs & ~0x3, &v);
	if (retval != PCISTS_OK)
		return retval;

	v = (v & ~(0xff << (8*(ofs&3)))) | ((0xff&val) << (8*(ofs&3)));

	return grpci2_cfg_w32(dev, ofs & ~0x3, v);
}

/* GRPCI2 PCI access routines, default to Little-endian PCI Bus */
struct pci_access_drv sample_grpci2_access = {
	.cfg =
	{
		grpci2_cfg_r8,
		grpci2_cfg_r16,
		grpci2_cfg_r32,
		grpci2_cfg_w8,
		grpci2_cfg_w16,
		grpci2_cfg_w32,
	},
	.io =
	{
		_ld8,
		_ld_le16,
		_ld_le32,
		_st8,
		_st_le16,
		_st_le32,
	},
	.memreg = &pci_memreg_sparc_le_ops,
	.translate = NULL,
};

struct pci_auto_setup sample_grpci2_cfg =
{
	.options = 0,

	/* PCI prefetchable Memory space (OPTIONAL) */
	.mem_start = 0,
	.mem_size = 0, /* 0 = Use MEMIO space for prefetchable mem BARs */

	/* PCI non-prefetchable Memory */
	memio_start = 0xA0000000, /* pci area */;
	memio_size = 0x10000000,

	/* PCI I/O space (OPTIONAL) */
	.io_start = 0x100; /* avoid PCI address 0 */
	.io_size = 0x10000 - 0x100; /* lower 64kB I/O 16 */

	/* Get System IRQ connected to a PCI line of a PCI device on bus0.
	 * The return IRQ value zero equals no IRQ (IRQ disabled).
	 */
	.irq_map = NULL,

	/* IRQ Bridge routing. Returns the interrupt pin (0..3 = A..D) that
	 * a device is connected to on parent bus.
	 */
	.irq_route = NULL,
};
