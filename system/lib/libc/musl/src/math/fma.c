#include <fenv.h>
#include "libm.h"

#if LDBL_MANT_DIG==64 && LDBL_MAX_EXP==16384
/* exact add, assumes exponent_x >= exponent_y */
static void add(long double *hi, long double *lo, long double x, long double y)
{
	long double r;

	r = x + y;
	*hi = r;
	r -= x;
	*lo = y - r;
}

/* exact mul, assumes no over/underflow */
static void mul(long double *hi, long double *lo, long double x, long double y)
{
	static const long double c = 1.0 + 0x1p32L;
	long double cx, xh, xl, cy, yh, yl;

	cx = c*x;
	xh = (x - cx) + cx;
	xl = x - xh;
	cy = c*y;
	yh = (y - cy) + cy;
	yl = y - yh;
	*hi = x*y;
	*lo = (xh*yh - *hi) + xh*yl + xl*yh + xl*yl;
}

/*
assume (long double)(hi+lo) == hi
return an adjusted hi so that rounding it to double (or less) precision is correct
*/
static long double adjust(long double hi, long double lo)
{
	union ldshape uhi, ulo;

	if (lo == 0)
		return hi;
	uhi.f = hi;
	if (uhi.i.m & 0x3ff)
		return hi;
	ulo.f = lo;
	if ((uhi.i.se & 0x8000) == (ulo.i.se & 0x8000))
		uhi.i.m++;
	else {
		/* handle underflow and take care of ld80 implicit msb */
		if (uhi.i.m << 1 == 0) {
			uhi.i.m = 0;
			uhi.i.se--;
		}
		uhi.i.m--;
	}
	return uhi.f;
}

/* adjusted add so the result is correct when rounded to double (or less) precision */
static long double dadd(long double x, long double y)
{
	add(&x, &y, x, y);
	return adjust(x, y);
}

/* adjusted mul so the result is correct when rounded to double (or less) precision */
static long double dmul(long double x, long double y)
{
	mul(&x, &y, x, y);
	return adjust(x, y);
}

static int getexp(long double x)
{
	union ldshape u;
	u.f = x;
	return u.i.se & 0x7fff;
}

double fma(double x, double y, double z)
{
	#pragma STDC FENV_ACCESS ON
	long double hi, lo1, lo2, xy;
	int round, ez, exy;

	/* handle +-inf,nan */
	if (!isfinite(x) || !isfinite(y))
		return x*y + z;
	if (!isfinite(z))
		return z;
	/* handle +-0 */
	if (x == 0.0 || y == 0.0)
		return x*y + z;
	round = fegetround();
	if (z == 0.0) {
		if (round == FE_TONEAREST)
			return dmul(x, y);
		return x*y;
	}

	/* exact mul and add require nearest rounding */
	/* spurious inexact exceptions may be raised */
	fesetround(FE_TONEAREST);
	mul(&xy, &lo1, x, y);
	exy = getexp(xy);
	ez = getexp(z);
	if (ez > exy) {
		add(&hi, &lo2, z, xy);
	} else if (ez > exy - 12) {
		add(&hi, &lo2, xy, z);
		if (hi == 0) {
			/*
			xy + z is 0, but it should be calculated with the
			original rounding mode so the sign is correct, if the
			compiler does not support FENV_ACCESS ON it does not
			know about the changed rounding mode and eliminates
			the xy + z below without the volatile memory access
			*/
			volatile double z_;
			fesetround(round);
			z_ = z;
			return (xy + z_) + lo1;
		}
	} else {
		/*
		ez <= exy - 12
		the 12 extra bits (1guard, 11round+sticky) are needed so with
			lo = dadd(lo1, lo2)
		elo <= ehi - 11, and we use the last 10 bits in adjust so
			dadd(hi, lo)
		gives correct result when rounded to double
		*/
		hi = xy;
		lo2 = z;
	}
	/*
	the result is stored before return for correct precision and exceptions

	one corner case is when the underflow flag should be raised because
	the precise result is an inexact subnormal double, but the calculated
	long double result is an exact subnormal double
	(so rounding to double does not raise exceptions)

	in nearest rounding mode dadd takes care of this: the last bit of the
	result is adjusted so rounding sees an inexact value when it should

	in non-nearest rounding mode fenv is used for the workaround
	*/
	fesetround(round);
	if (round == FE_TONEAREST)
		z = dadd(hi, dadd(lo1, lo2));
	else {
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
		int e = fetestexcept(FE_INEXACT);
		feclearexcept(FE_INEXACT);
#endif
		z = hi + (lo1 + lo2);
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
		if (getexp(z) < 0x3fff-1022 && fetestexcept(FE_INEXACT))
			feraiseexcept(FE_UNDERFLOW);
		else if (e)
			feraiseexcept(FE_INEXACT);
#endif
	}
	return z;
}
#else
/* origin: FreeBSD /usr/src/lib/msun/src/s_fma.c */
/*-
 * Copyright (c) 2005-2011 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * A struct dd represents a floating-point number with twice the precision
 * of a double.  We maintain the invariant that "hi" stores the 53 high-order
 * bits of the result.
 */
struct dd {
	double hi;
	double lo;
};

/*
 * Compute a+b exactly, returning the exact result in a struct dd.  We assume
 * that both a and b are finite, but make no assumptions about their relative
 * magnitudes.
 */
static inline struct dd dd_add(double a, double b)
{
	struct dd ret;
	double s;

	ret.hi = a + b;
	s = ret.hi - a;
	ret.lo = (a - (ret.hi - s)) + (b - s);
	return (ret);
}

/*
 * Compute a+b, with a small tweak:  The least significant bit of the
 * result is adjusted into a sticky bit summarizing all the bits that
 * were lost to rounding.  This adjustment negates the effects of double
 * rounding when the result is added to another number with a higher
 * exponent.  For an explanation of round and sticky bits, see any reference
 * on FPU design, e.g.,
 *
 *     J. Coonen.  An Implementation Guide to a Proposed Standard for
 *     Floating-Point Arithmetic.  Computer, vol. 13, no. 1, Jan 1980.
 */
static inline double add_adjusted(double a, double b)
{
	struct dd sum;
	union {double f; uint64_t i;} uhi, ulo;

	sum = dd_add(a, b);
	if (sum.lo != 0) {
		uhi.f = sum.hi;
		if ((uhi.i & 1) == 0) {
			/* hibits += (int)copysign(1.0, sum.hi * sum.lo) */
			ulo.f = sum.lo;
			uhi.i += 1 - ((uhi.i ^ ulo.i) >> 62);
			sum.hi = uhi.f;
		}
	}
	return (sum.hi);
}

/*
 * Compute ldexp(a+b, scale) with a single rounding error. It is assumed
 * that the result will be subnormal, and care is taken to ensure that
 * double rounding does not occur.
 */
static inline double add_and_denormalize(double a, double b, int scale)
{
	struct dd sum;
	union {double f; uint64_t i;} uhi, ulo;
	int bits_lost;

	sum = dd_add(a, b);

	/*
	 * If we are losing at least two bits of accuracy to denormalization,
	 * then the first lost bit becomes a round bit, and we adjust the
	 * lowest bit of sum.hi to make it a sticky bit summarizing all the
	 * bits in sum.lo. With the sticky bit adjusted, the hardware will
	 * break any ties in the correct direction.
	 *
	 * If we are losing only one bit to denormalization, however, we must
	 * break the ties manually.
	 */
	if (sum.lo != 0) {
		uhi.f = sum.hi;
		bits_lost = -((int)(uhi.i >> 52) & 0x7ff) - scale + 1;
		if ((bits_lost != 1) ^ (int)(uhi.i & 1)) {
			/* hibits += (int)copysign(1.0, sum.hi * sum.lo) */
			ulo.f = sum.lo;
			uhi.i += 1 - (((uhi.i ^ ulo.i) >> 62) & 2);
			sum.hi = uhi.f;
		}
	}
	return scalbn(sum.hi, scale);
}

/*
 * Compute a*b exactly, returning the exact result in a struct dd.  We assume
 * that both a and b are normalized, so no underflow or overflow will occur.
 * The current rounding mode must be round-to-nearest.
 */
static inline struct dd dd_mul(double a, double b)
{
	static const double split = 0x1p27 + 1.0;
	struct dd ret;
	double ha, hb, la, lb, p, q;

	p = a * split;
	ha = a - p;
	ha += p;
	la = a - ha;

	p = b * split;
	hb = b - p;
	hb += p;
	lb = b - hb;

	p = ha * hb;
	q = ha * lb + la * hb;

	ret.hi = p + q;
	ret.lo = p - ret.hi + q + la * lb;
	return (ret);
}

/*
 * Fused multiply-add: Compute x * y + z with a single rounding error.
 *
 * We use scaling to avoid overflow/underflow, along with the
 * canonical precision-doubling technique adapted from:
 *
 *      Dekker, T.  A Floating-Point Technique for Extending the
 *      Available Precision.  Numer. Math. 18, 224-242 (1971).
 *
 * This algorithm is sensitive to the rounding precision.  FPUs such
 * as the i387 must be set in double-precision mode if variables are
 * to be stored in FP registers in order to avoid incorrect results.
 * This is the default on FreeBSD, but not on many other systems.
 *
 * Hardware instructions should be used on architectures that support it,
 * since this implementation will likely be several times slower.
 */
double fma(double x, double y, double z)
{
	#pragma STDC FENV_ACCESS ON
	double xs, ys, zs, adj;
	struct dd xy, r;
	int oround;
	int ex, ey, ez;
	int spread;

	/*
	 * Handle special cases. The order of operations and the particular
	 * return values here are crucial in handling special cases involving
	 * infinities, NaNs, overflows, and signed zeroes correctly.
	 */
	if (!isfinite(x) || !isfinite(y))
		return (x * y + z);
	if (!isfinite(z))
		return (z);
	if (x == 0.0 || y == 0.0)
		return (x * y + z);
	if (z == 0.0)
		return (x * y);

	xs = frexp(x, &ex);
	ys = frexp(y, &ey);
	zs = frexp(z, &ez);
	oround = fegetround();
	spread = ex + ey - ez;

	/*
	 * If x * y and z are many orders of magnitude apart, the scaling
	 * will overflow, so we handle these cases specially.  Rounding
	 * modes other than FE_TONEAREST are painful.
	 */
	if (spread < -DBL_MANT_DIG) {
#ifdef FE_INEXACT
		feraiseexcept(FE_INEXACT);
#endif
#ifdef FE_UNDERFLOW
		if (!isnormal(z))
			feraiseexcept(FE_UNDERFLOW);
#endif
		switch (oround) {
		default: /* FE_TONEAREST */
			return (z);
#ifdef FE_TOWARDZERO
		case FE_TOWARDZERO:
			if (x > 0.0 ^ y < 0.0 ^ z < 0.0)
				return (z);
			else
				return (nextafter(z, 0));
#endif
#ifdef FE_DOWNWARD
		case FE_DOWNWARD:
			if (x > 0.0 ^ y < 0.0)
				return (z);
			else
				return (nextafter(z, -INFINITY));
#endif
#ifdef FE_UPWARD
		case FE_UPWARD:
			if (x > 0.0 ^ y < 0.0)
				return (nextafter(z, INFINITY));
			else
				return (z);
#endif
		}
	}
	if (spread <= DBL_MANT_DIG * 2)
		zs = scalbn(zs, -spread);
	else
		zs = copysign(DBL_MIN, zs);

	fesetround(FE_TONEAREST);

	/*
	 * Basic approach for round-to-nearest:
	 *
	 *     (xy.hi, xy.lo) = x * y           (exact)
	 *     (r.hi, r.lo)   = xy.hi + z       (exact)
	 *     adj = xy.lo + r.lo               (inexact; low bit is sticky)
	 *     result = r.hi + adj              (correctly rounded)
	 */
	xy = dd_mul(xs, ys);
	r = dd_add(xy.hi, zs);

	spread = ex + ey;

	if (r.hi == 0.0) {
		/*
		 * When the addends cancel to 0, ensure that the result has
		 * the correct sign.
		 */
		fesetround(oround);
		volatile double vzs = zs; /* XXX gcc CSE bug workaround */
		return xy.hi + vzs + scalbn(xy.lo, spread);
	}

	if (oround != FE_TONEAREST) {
		/*
		 * There is no need to worry about double rounding in directed
		 * rounding modes.
		 * But underflow may not be raised properly, example in downward rounding:
		 * fma(0x1.000000001p-1000, 0x1.000000001p-30, -0x1p-1066)
		 */
		double ret;
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
		int e = fetestexcept(FE_INEXACT);
		feclearexcept(FE_INEXACT);
#endif
		fesetround(oround);
		adj = r.lo + xy.lo;
		ret = scalbn(r.hi + adj, spread);
#if defined(FE_INEXACT) && defined(FE_UNDERFLOW)
		if (ilogb(ret) < -1022 && fetestexcept(FE_INEXACT))
			feraiseexcept(FE_UNDERFLOW);
		else if (e)
			feraiseexcept(FE_INEXACT);
#endif
		return ret;
	}

	adj = add_adjusted(r.lo, xy.lo);
	if (spread + ilogb(r.hi) > -1023)
		return scalbn(r.hi + adj, spread);
	else
		return add_and_denormalize(r.hi, adj, spread);
}
#endif
