#ifndef _INCLUDE_LEONCOMPAT_h
#define _INCLUDE_LEONCOMPAT_h

#include <asm-leon/leon.h>

#ifndef __ASSEMBLER__

#define LEONCOMPAT_VERSION _leon_version
#define LEONCOMPAT_VERSION_ISLEON3 (LEONCOMPAT_VERSION == 3)
extern int _leon_version;

#endif /* __ASSEMBLER__ */

#endif /* !_INCLUDE_LEONCOMPAT_h */

