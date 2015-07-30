#include "libm.h"

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
long double roundl(long double x)
{
	return round(x);
}
#elif (LDBL_MANT_DIG == 64 || LDBL_MANT_DIG == 113) && LDBL_MAX_EXP == 16384
#if LDBL_MANT_DIG == 64
#define TOINT 0x1p63
#elif LDBL_MANT_DIG == 113
#define TOINT 0x1p112
#endif
long double roundl(long double x)
{
	union ldshape u = {x};
	int e = u.i.se & 0x7fff;
	long double y;

	if (e >= 0x3fff+LDBL_MANT_DIG-1)
		return x;
	if (u.i.se >> 15)
		x = -x;
	if (e < 0x3fff-1) {
		FORCE_EVAL(x + TOINT);
		return 0*u.f;
	}
	y = x + TOINT - TOINT - x;
	if (y > 0.5)
		y = y + x - 1;
	else if (y <= -0.5)
		y = y + x + 1;
	else
		y = y + x;
	if (u.i.se >> 15)
		y = -y;
	return y;
}
#endif
