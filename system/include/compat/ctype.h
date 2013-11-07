#ifndef  _COMPAT_CTYPE_H_
#define  _COMPAT_CTYPE_H_

#define _CTYPE_A  0x00000400   /* Alpha */
#define _CTYPE_C  0x00000002   /* Control */
#define _CTYPE_D  0x00000800   /* Digit */
#define _CTYPE_L  0x00000200   /* Lower */
#define _CTYPE_P  0x00000004   /* Punct */
#define _CTYPE_S  0x00002000   /* Space */
#define _CTYPE_U  0x00000100   /* Upper */
#define _CTYPE_X  0x00001000   /* X digit */
#define _CTYPE_B  0x00000001   /* Blank */
#define _CTYPE_R  0x00004000   /* Print */

#include_next <ctype.h>

/* We undef these until libcxx is fixed. Without this,
   some things can fail to compile correctly, like
   Boost. Issue #1716. */

#undef isalpha
#undef isblank
#undef iscntrl
#undef isdigit
#undef isgraph
#undef islower
#undef isprint
#undef ispunct
#undef isspace
#undef isupper
#undef isxdigit
#undef isctype

#endif /* _COMPAT_CTYPE_H_ */
