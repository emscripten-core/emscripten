#include <xmmintrin.h>
#include <stdio.h>

class vec
{
public:
    __m128 v;

    void mul(float s)
    {
        __m128 vs = _mm_load_ss(&s);
        // __m128 vs = _mm_set_ss(s); // _mm_set_ss(s) instead of _mm_load_ss(&s) avoids the problem somehow..?
        v = _mm_mul_ps(v, vs);
    }
};

int main()
{
    vec v;
    v.v = _mm_set_ps(8,8,8,8);
    v.mul(0.5f);

    float f[4];
    _mm_storeu_ps(f, v.v);
    printf("%f %f %f %f\n", f[0], f[1], f[2], f[3]);
}

