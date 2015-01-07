#include "libm.h"

float roundf(float x)
{
	union {float f; uint32_t i;} u = {x};
	int e = u.i >> 23 & 0xff;
	float_t y;

	if (e >= 0x7f+23)
		return x;
	if (u.i >> 31)
		x = -x;
	if (e < 0x7f-1) {
		FORCE_EVAL(x + 0x1p23f);
		return 0*u.f;
	}
	y = (float)(x + 0x1p23f) - 0x1p23f - x;
	if (y > 0.5f)
		y = y + x - 1;
	else if (y <= -0.5f)
		y = y + x + 1;
	else
		y = y + x;
	if (u.i >> 31)
		y = -y;
	return y;
}
