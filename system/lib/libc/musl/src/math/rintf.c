#include <math.h>
#include <stdint.h>

float rintf(float x)
{
	union {float f; uint32_t i;} u = {x};
	int e = u.i>>23 & 0xff;
	int s = u.i>>31;
	float_t y;

	if (e >= 0x7f+23)
		return x;
	if (s)
		y = (float)(x - 0x1p23f) + 0x1p23f;
	else
		y = (float)(x + 0x1p23f) - 0x1p23f;
	if (y == 0)
		return s ? -0.0f : 0.0f;
	return y;
}
