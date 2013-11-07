#ifndef  _COMPAT_WCHAR_H_
#define  _COMPAT_WCHAR_H_

#include_next <wchar.h>

/* We undef these until libcxx is fixed. Without this,
   some things can fail to compile correctly, like
   Boost. Issue #1716. */

#undef iswalpha
#undef iswblank
#undef iswcntrl
#undef iswdigit
#undef iswgraph
#undef iswlower
#undef iswprint
#undef iswpunct
#undef iswspace
#undef iswupper
#undef iswxdigit
#undef iswctype

#endif /* _COMPAT_WCHAR_H_ */
