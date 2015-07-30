#include <xmmintrin.h>
#include <stdio.h>
#include <string>

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
    __m128 m1 = _mm_set1_ps(1.f);
    __m128 m2 = _mm_set1_ps(2.f);
    __m128 ret = _mm_add_ps(m1, m2);
    to_str(m1).c_str(), to_str(m2).c_str(), to_str(ret).c_str();
}

