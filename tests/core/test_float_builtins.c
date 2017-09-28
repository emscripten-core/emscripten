#include <stdio.h>

// Prints the float/double/long double versions of the given function
// e.g.: TEST(fmax) prints fmaxf : fmax : fmaxl
#define TEST(func)                    \
    printf("%f : %f : %Lf\n",         \
        __builtin_##func##f(f1, f2),  \
        __builtin_##func(d1, d2),     \
        __builtin_##func##l(l1, l2));

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
