#include <fenv.h>

/* __fesetround wrapper for arch independent argument check */

int __fesetround(int);

int fesetround(int r)
{
	if (r & ~(
		FE_TONEAREST
#ifdef FE_DOWNWARD
		|FE_DOWNWARD
#endif
#ifdef FE_UPWARD
		|FE_UPWARD
#endif
#ifdef FE_TOWARDZERO
		|FE_TOWARDZERO
#endif
		))
		return -1;
	return __fesetround(r);
}
