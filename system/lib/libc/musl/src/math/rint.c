#include <math.h>
#include <stdint.h>

double rint(double x)
{
	union {double f; uint64_t i;} u = {x};
	int e = u.i>>52 & 0x7ff;
	int s = u.i>>63;
	double_t y;

	if (e >= 0x3ff+52)
		return x;
	if (s)
		y = (double)(x - 0x1p52) + 0x1p52;
	else
		y = (double)(x + 0x1p52) - 0x1p52;
	if (y == 0)
		return s ? -0.0 : 0;
	return y;
}
