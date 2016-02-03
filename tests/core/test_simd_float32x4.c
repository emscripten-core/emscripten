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
    bool32x4 b = emscripten_int32x4_set(0, -1, 0, -1);
    DUMP(emscripten_float32x4_select(b, v, w));
    DUMP(emscripten_float32x4_replaceLane(v, 0, 9.f));
    DUMP(emscripten_float32x4_replaceLane(v, 1, -3.f));
    DUMP(emscripten_float32x4_replaceLane(v, 2, 0.f));
    DUMP(emscripten_float32x4_replaceLane(v, 3, -0.f));
    // TODO: emscripten_float32x4_store
    // TODO: emscripten_float32x4_store1
    // TODO: emscripten_float32x4_store2
    // TODO: emscripten_float32x4_store3
    // TODO: emscripten_float32x4_load
    // TODO: emscripten_float32x4_load1
    // TODO: emscripten_float32x4_load2
    // TODO: emscripten_float32x4_load3
    // TODO: emscripten_float32x4_fromFloat64x2Bits
    // TODO: emscripten_float32x4_fromInt32x4Bits
    // TODO: emscripten_float32x4_fromUint32x4Bits
    // TODO: emscripten_float32x4_fromInt16x8Bits
    // TODO: emscripten_float32x4_fromUint16x8Bits
    // TODO: emscripten_float32x4_fromInt8x16Bits
    // TODO: emscripten_float32x4_fromUint8x16Bits
    // TODO: emscripten_float32x4_fromInt32x4
    // TODO: emscripten_float32x4_fromUint32x4
    DUMP(emscripten_float32x4_swizzle(v, 0, 1, 2, 3));
    DUMP(emscripten_float32x4_swizzle(v, 3, 2, 1, 0));
    DUMP(emscripten_float32x4_swizzle(v, 0, 0, 0, 0));
    DUMP(emscripten_float32x4_swizzle(v, 0, 3, 0, 3));
    DUMP(emscripten_float32x4_swizzle(v, 3, 3, 3, 3));
    float32x4 z = emscripten_float32x4_set(-5.f, 20.f, 14.f, 9.f);
    DUMP(z);
    DUMP(emscripten_float32x4_shuffle(v, z, 0, 0, 0, 0));
    DUMP(emscripten_float32x4_shuffle(v, z, 4, 4, 4, 4));
    DUMP(emscripten_float32x4_shuffle(v, z, 7, 7, 7, 7));
    DUMP(emscripten_float32x4_shuffle(v, z, 0, 2, 4, 6));
    DUMP(emscripten_float32x4_shuffle(v, z, 7, 0, 3, 5));

    printf("Done!\n");
}
