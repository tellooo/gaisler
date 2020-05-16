/*
 *  byteorder.h  - Endian conversion for SPARC. SPARC is big endian only.
 *
 *  COPYRIGHT (c) 2011
 *  Aeroflex Gaisler.
 *
 *  The license and distribution terms for this file may be
 *  found in the file LICENSE in this distribution or at
 *  http://www.rtems.com/license/LICENSE.
 */

#ifndef _LIBCPU_BYTEORDER_H
#define _LIBCPU_BYTEORDER_H

#include <stdint.h>

/*
 *  CPU_swap_u32
 *
 *  The following routine swaps the endian format of an unsigned int.
 *  It must be static because it is referenced indirectly.
 *
 *  This version will work on any processor, but if you come across a better
 *  way for the SPARC PLEASE use it.  The most common way to swap a 32-bit
 *  entity as shown below is not any more efficient on the SPARC.
 *
 *     swap least significant two bytes with 16-bit rotate
 *     swap upper and lower 16-bits
 *     swap most significant two bytes with 16-bit rotate
 *
 *  It is not obvious how the SPARC can do significantly better than the
 *  generic code.  gcc 2.7.0 only generates about 12 instructions for the
 *  following code at optimization level four (i.e. -O4).
 */

static inline uint32_t CPU_swap_u32(
  uint32_t value
)
{
  uint32_t   byte1, byte2, byte3, byte4, swapped;

  byte4 = (value >> 24) & 0xff;
  byte3 = (value >> 16) & 0xff;
  byte2 = (value >> 8)  & 0xff;
  byte1 =  value        & 0xff;

  swapped = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;
  return( swapped );
}

#define CPU_swap_u16( value ) \
  (((value&0xff) << 8) | ((value >> 8)&0xff))


#ifdef __cplusplus
extern "C" {
#endif

#ifndef RTEMS_INLINE_ROUTINE
#define RTEMS_INLINE_ROUTINE static inline
#endif

RTEMS_INLINE_ROUTINE uint16_t ld_le16(volatile uint16_t *addr)
{
	return CPU_swap_u16(*addr);
}

RTEMS_INLINE_ROUTINE void st_le16(volatile uint16_t *addr, uint16_t val)
{
	*addr = CPU_swap_u16(val);
}

RTEMS_INLINE_ROUTINE uint32_t ld_le32(volatile uint32_t *addr)
{
	return CPU_swap_u32(*addr);
}

RTEMS_INLINE_ROUTINE void st_le32(volatile uint32_t *addr, uint32_t val)
{
	*addr = CPU_swap_u32(val);
}

RTEMS_INLINE_ROUTINE uint16_t ld_be16(volatile uint16_t *addr)
{
	return *addr;
}

RTEMS_INLINE_ROUTINE void st_be16(volatile uint16_t *addr, uint16_t val)
{
	*addr = val;
}

RTEMS_INLINE_ROUTINE uint32_t ld_be32(volatile uint32_t *addr)
{
	return *addr;
}

RTEMS_INLINE_ROUTINE void st_be32(volatile uint32_t *addr, uint32_t val)
{
	*addr = val;
}

#ifdef __cplusplus
}
#endif

#endif
