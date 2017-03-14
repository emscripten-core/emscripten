#include <stddef.h>
#include "libc.h"

__attribute__((__visibility__("hidden")))
ptrdiff_t __tlsdesc_static(), __tlsdesc_dynamic();

ptrdiff_t __tlsdesc_static()
{
	return 0;
}

weak_alias(__tlsdesc_static, __tlsdesc_dynamic);
