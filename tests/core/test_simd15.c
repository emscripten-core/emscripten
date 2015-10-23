#include <emmintrin.h>
#include <stdio.h>
#include <stdint.h>

__m128i set_64x2(int64_t a, int64_t b) { union { int64_t x[2]; __m128i m; } u; u.x[0] = a; u.x[1] = b; return u.m; }
int64_t get_64x2_lo(__m128i m) { union { int64_t x[2]; __m128i m; } u; u.m = m; return u.x[0]; }
int64_t get_64x2_hi(__m128i m) { union { int64_t x[2]; __m128i m; } u; u.m = m; return u.x[1]; }

void test1()
{
    __m128i a = set_64x2(1, 2);
    __m128i b = set_64x2(3, 4);
    __m128i c = _mm_add_epi64(a, b);
    printf("%lld %lld\n", get_64x2_lo(c), get_64x2_hi(c));
}

typedef int m128i __attribute__((__vector_size__(16)));

m128i __attribute__((noinline)) add_epi16(m128i a, m128i b)
{
    union {
        signed short x[8];
        m128i m;
    } src, src2, dst;
    src.m = a;
    src2.m = b;
    dst.x[0] = src.x[0] + src2.x[0];
    return dst.m;
}

void test2()
{
    m128i a = (m128i){ 1, 2, 3, 4 };
    m128i b = add_epi16(a, a);
    printf("%d\n", b[0]);
}


int main() {
    test1();
    test2();
}

