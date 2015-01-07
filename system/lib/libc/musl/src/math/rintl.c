#include "libm.h"

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double rintl(long double x)
{
	return rint(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#define TOINT 0x1p63
#elif LDBL_MANT_DIG == 113
#define TOINT 0x1p112
#endif
long double rintl(long double x)
{
	union ldshape u = {x};
	int e = u.i.se & 0x7fff;
	int s = u.i.se >> 15;
	long double y;

	if (e >= 0x3fff+LDBL_MANT_DIG-1)
		return x;
	if (s)
		y = x - TOINT + TOINT;
	else
		y = x + TOINT - TOINT;
	if (y == 0)
		return 0*x;
	return y;
}
#endif
