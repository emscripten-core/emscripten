/* $NetBSD: complex.h,v 1.3 2010/09/15 16:11:30 christos Exp $ */

/*
 * Written by Matthias Drochner.
 * Public domain.
 */

#ifndef	_COMPLEX_H
#define	_COMPLEX_H

#define complex _Complex
#define _Complex_I 1.0fi
#define I _Complex_I

#include <sys/cdefs.h>

__BEGIN_DECLS

/* 7.3.5 Trigonometric functions */
/* 7.3.5.1 The cacos functions */
double complex cacos(double complex);
float complex cacosf(float complex);

/* 7.3.5.2 The casin functions */
double complex casin(double complex);
float complex casinf(float complex);

/* 7.3.5.1 The catan functions */
double complex catan(double complex);
float complex catanf(float complex);

/* 7.3.5.1 The ccos functions */
double complex ccos(double complex);
float complex ccosf(float complex);

/* 7.3.5.1 The csin functions */
double complex csin(double complex);
float complex csinf(float complex);

/* 7.3.5.1 The ctan functions */
double complex ctan(double complex);
float complex ctanf(float complex);

/* 7.3.6 Hyperbolic functions */
/* 7.3.6.1 The cacosh functions */
double complex cacosh(double complex);
float complex cacoshf(float complex);

/* 7.3.6.2 The casinh functions */
double complex casinh(double complex);
float complex casinhf(float complex);

/* 7.3.6.3 The catanh functions */
double complex catanh(double complex);
float complex catanhf(float complex);

/* 7.3.6.4 The ccosh functions */
double complex ccosh(double complex);
float complex ccoshf(float complex);

/* 7.3.6.5 The csinh functions */
double complex csinh(double complex);
float complex csinhf(float complex);

/* 7.3.6.6 The ctanh functions */
double complex ctanh(double complex);
float complex ctanhf(float complex);

/* 7.3.7 Exponential and logarithmic functions */
/* 7.3.7.1 The cexp functions */
double complex cexp(double complex);
float complex cexpf(float complex);

/* 7.3.7.2 The clog functions */
double complex clog(double complex);
float complex clogf(float complex);

/* 7.3.8 Power and absolute-value functions */
/* 7.3.8.1 The cabs functions */
/*#ifndef __LIBM0_SOURCE__
/* avoid conflict with historical cabs(struct complex) */
/* double cabs(double complex) __RENAME(__c99_cabs);
   float cabsf(float complex) __RENAME(__c99_cabsf);
   #endif
*/
double cabs(double complex) ;
float cabsf(float complex) ;

/* 7.3.8.2 The cpow functions */
double complex cpow(double complex, double complex);
float complex cpowf(float complex, float complex);

/* 7.3.8.3 The csqrt functions */
double complex csqrt(double complex);
float complex csqrtf(float complex);

/* 7.3.9 Manipulation functions */
/* 7.3.9.1 The carg functions */ 
double carg(double complex);
float cargf(float complex);

/* 7.3.9.2 The cimag functions */
double cimag(double complex);
float cimagf(float complex);
/*long double cimagl(long double complex); */

/* 7.3.9.3 The conj functions */
double complex conj(double complex);
float complex conjf(float complex);
/*long double complex conjl(long double complex); */

/* 7.3.9.4 The cproj functions */
double complex cproj(double complex);
float complex cprojf(float complex);
/*long double complex cprojl(long double complex); */

/* 7.3.9.5 The creal functions */
double creal(double complex);
float crealf(float complex);
/*long double creall(long double complex); */

__END_DECLS

#endif	/* ! _COMPLEX_H */
