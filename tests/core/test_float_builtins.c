#include <stdio.h>
#include <math.h>

int main() {
    float f1 = 0.1234f;
    float f2 = 0.5678f;
    double d1 = 1.0101;
    double d2 = 0.10101;
    printf(
        "%f\n%f\n%f\n%f\n",
        __builtin_fmaxf(f1, f2),
        __builtin_fminf(f1, f2),
        __builtin_fmax(d1, d2),
        __builtin_fmin(d1, d2)
    );
    return 0;
}
