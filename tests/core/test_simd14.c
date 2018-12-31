#include <stdio.h>
#include <xmmintrin.h>

void __attribute__((noinline)) foo(__m128 vec, __m128 &outLength)
{
    outLength = vec;
}

int main()
{
    puts("before");
    __m128 x;
    foo(_mm_set1_ps(1.f), x);
    printf("%.2f", x[0]);
}

