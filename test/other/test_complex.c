/*
 * Copyright 2016 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

#include <complex.h>
#include <stdio.h>

int main(int argc, char**argv)
{
   float complex z1 = 1.0 + 3.0 * I;
   printf("value = real %.2f imag %.2f\n",creal(z1),cimag(z1));
   float abs_value = cabsf(z1);
   printf("abs = %.2f\n",abs_value);
   float complex z2 =  conjf(z1); 
   printf("value = real %.2f imag %.2f\n",creal(z2),cimag(z2));
   float complex z3 =  cexpf(z1); 
   printf("value = real %.2f imag %.2f\n",creal(z3),cimag(z3));
   float complex z4 =  conj(z1); 
   printf("value = real %.2f imag %.2f\n",creal(z4),cimag(z4));
   float complex z5 =  cargf(z1); 
   printf("value = real %.2f imag %.2f\n",creal(z5),cimag(z5));
   float complex z6 = 0.5 + 0.5 * I;
   float complex z7 = 0.5 - 0.5 * I;
   float complex z8 = z6 * z7;
   printf("value = real %.2f imag %.2f\n",creal(z8),cimag(z8));
   float complex z9 = z6 / z7;
   printf("value = real %.2f imag %.2f\n",creal(z9),cimag(z9));
   return 0;
}
