#include <xmmintrin.h>
#include <stdio.h>
#include <string>

__m128 getSomeSIMD() {
    union { __m128 m; float val[4]; } u;
    u.val[0] = 1;
    u.val[1] = 3.14159;
    u.val[2] = -124234234.5;
    u.val[3] = 99;
    return u.m;
}

__attribute__((noinline)) std::string to_str(__m128 m)
{
    union { __m128 m; float val[4]; } u;
    u.m = m;
    char str[256];
    sprintf(str, "[%f,%f,%f,%f]", u.val[3], u.val[2], u.val[1], u.val[0]);
    printf("%s\n", str);
    return "?";
}

int main()
{
    // part 1
    __m128 m1 = _mm_set1_ps(1.f);
    __m128 m2 = _mm_set1_ps(2.f);
    __m128 ret = _mm_add_ps(m1, m2);
    to_str(m1).c_str(), to_str(m2).c_str(), to_str(ret).c_str();

    // part 2
    typedef __m128 (*type)();
    volatile type ptr;
    ptr = &getSomeSIMD;
    to_str(ptr());
}

