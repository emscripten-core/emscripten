#include <stdio.h>
#include <math.h>

#define TEST(op)                    \
    printf("%f : %f : %Lf\n",       \
        __builtin_##op##f(f1, f2),  \
        __builtin_##op(d1, d2),     \
        __builtin_##op##l(l1, l2));

int main() {
    float f1 = 0.1234f;
    float f2 = 0.5678f;
    double d1 = 1.0101;
    double d2 = 0.10101;
    long double l1 = 12.0123L;
    long double l2 = 21.3201L;
    puts("***start***");
    TEST(fmax)
    TEST(fmin)
    TEST(fmod)
    puts("***end***");
    return 0;
}
