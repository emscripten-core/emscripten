#define _BSD_SOURCE 1
#define _XOPEN_SOURCE 700
#include <stdint.h>
#include <stdio.h>
#include <fenv.h>
#include <float.h>
#include <math.h>

#define RN 0
#define T(...) {__FILE__, __LINE__, __VA_ARGS__},
#define POS const char *file; int line;
struct f_fi {POS int r; float x; float y; float dy; long long i; int e; };

#define DIVBYZERO 0
#define INEXACT 0
#define INVALID 0
#define OVERFLOW 0
#define UNDERFLOW 0

#define inf INFINITY
#define nan NAN

static struct f_fi t[] = {
T(RN,   -0x1.02239f3c6a8f1p+3,   -0x1.0120f61b63d5ep+3,   0x1.89ccc4p-6,          -1, INEXACT)
T(RN,    0x1.161868e18bc67p+2,    0x1.1ef3b263fd60bp+1,  -0x1.6d0264p-3,           1, INEXACT)
T(RN,   -0x1.0c34b3e01e6e7p+3,   -0x1.46d73255263d9p+3,   0x1.e0ec76p-3,          -1, INEXACT)
T(RN,   -0x1.a206f0a19dcc4p+2,   -0x1.9c91f19ac48c5p+2,   0x1.c2a38cp-2,          -1, INEXACT)
T(RN,    0x1.288bbb0d6a1e6p+3,    0x1.65c60768fcc11p+3,   0x1.2f22c2p-2,           1, INEXACT)
T(RN,    0x1.52efd0cd80497p-1,    0x1.3cc760be720b3p-2,   0x1.0527e2p-2,           1, INEXACT)
T(RN,   -0x1.a05cc754481d1p-2,    0x1.4ef387fea1014p+0,  -0x1.c3b036p-2,          -1, INEXACT)
T(RN,    0x1.1f9ef934745cbp-1,    0x1.d6f0efacc5699p-2,   0x1.c0b0a8p-2,           1, INEXACT)
T(RN,    0x1.8c5db097f7442p-1,    0x1.6c1a14cf91533p-3,   0x1.16f4cap-5,           1, INEXACT)
T(RN,   -0x1.5b86ea8118a0ep-1,    0x1.695b1e0a0a59ep+0,   0x1.ada69ep-2,          -1, INEXACT)
T(RN,                  0x0p+0,                     inf,          0x0p+0,           1, DIVBYZERO)
/* T(RN,                 -0x0p+0,                     inf,          0x0p+0,          -1, DIVBYZERO) This one fails in native as well */
T(RN,                  0x1p+0,                  0x0p+0,          0x0p+0,           1, 0)
T(RN,                 -0x1p+0,                     inf,          0x0p+0,           1, DIVBYZERO)
T(RN,                  0x1p+1,                  0x0p+0,          0x0p+0,           1, 0)
T(RN,                 -0x1p+1,                     inf,          0x0p+0,           1, DIVBYZERO)
T(RN,                     inf,                     inf,          0x0p+0,           1, 0)
T(RN,                    -inf,                     inf,          0x0p+0,          -1, 0)
T(RN,                     nan,                     nan,          0x0p+0,           1, 0)
};

static int eulpf(float x)
{
  union { float f; uint32_t i; } u = { x };
  int e = u.i>>23 & 0xff;

  if (!e)
    e++;
  return e - 0x7f - 23;
}

static int checkulp(float d, int r)
{
  // TODO: we only care about >=1.5 ulp errors for now, should be 1.0
  if (r == RN)
    return fabsf(d) < 1.5;
  return 1;
}

static float ulperrf(float got, float want, float dwant)
{
  if (isnan(got) && isnan(want))
    return 0;
  if (got == want) {
    if (signbit(got) == signbit(want))
      return dwant;
    return INFINITY;
  }
  if (isinf(got)) {
    got = copysignf(0x1p127, got);
    want *= 0.5;
  }
  return scalbn(got - want, -eulpf(want)) + dwant;
}

int main(void)
{
	int yi;
	double y;
	float d;
	int e, i, err = 0;
	struct f_fi *p;

	for (i = 0; i < sizeof t/sizeof *t; i++) {
		p = t + i;

		if (p->r < 0)
			continue;
		y = lgammaf(p->x);
		yi = signgam;

    printf("%g,%d\n", y, yi);

		d = ulperrf(y, p->y, p->dy);
		if (!checkulp(d, p->r) || (!isnan(p->x) && p->x!=-inf && !(p->e&DIVBYZERO) && yi != p->i)) {
      /* printf("%s:%d: %d lgammaf(%g) want %g,%lld got %g,%d ulperr %.3f = %g + %g\n",
				p->file, p->line, p->r, p->x, p->y, p->i, y, yi, d, d-p->dy, p->dy); */
			err++;
		}
	}
	return !!err;
}
