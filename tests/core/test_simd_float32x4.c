#include <emscripten/vector.h>
#include <stdio.h>

void dump(const char *name, float32x4 vec)
{
    printf("%s: %f %f %f %f\n", name, emscripten_float32x4_extractLane(vec, 0), emscripten_float32x4_extractLane(vec, 1), emscripten_float32x4_extractLane(vec, 2), emscripten_float32x4_extractLane(vec, 3));
}
#define DUMP(V) dump(#V, (V))

int main()
{
    float32x4 v = emscripten_float32x4_set(-1.f, 0.f, 1.f, 3.5f);
    DUMP(v);
    float32x4 w = emscripten_float32x4_splat(2.f);
    DUMP(w);
    DUMP(emscripten_float32x4_add(v, w));
    DUMP(emscripten_float32x4_sub(v, w));
    DUMP(emscripten_float32x4_mul(v, w));
    DUMP(emscripten_float32x4_div(v, w));
    DUMP(emscripten_float32x4_max(v, w));
    DUMP(emscripten_float32x4_min(v, w));
    DUMP(emscripten_float32x4_maxNum(v, w));
    DUMP(emscripten_float32x4_minNum(v, w));
    DUMP(emscripten_float32x4_neg(v));
    DUMP(emscripten_float32x4_sqrt(v));
    DUMP(emscripten_float32x4_reciprocalApproximation(v));
    DUMP(emscripten_float32x4_reciprocalSqrtApproximation(v));
    DUMP(emscripten_float32x4_abs(v));
    DUMP(emscripten_float32x4_and(v, w));
    DUMP(emscripten_float32x4_xor(v, w));
    DUMP(emscripten_float32x4_or(v, w));
    DUMP(emscripten_float32x4_not(v));
    DUMP(emscripten_float32x4_lessThan(v, w));
    DUMP(emscripten_float32x4_lessThanOrEqual(v, w));
    DUMP(emscripten_float32x4_greaterThan(v, w));
    DUMP(emscripten_float32x4_greaterThanOrEqual(v, w));
    DUMP(emscripten_float32x4_equal(v, w));
    DUMP(emscripten_float32x4_notEqual(v, w));
}

