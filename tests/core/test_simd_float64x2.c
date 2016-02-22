#include <emscripten/vector.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <assert.h>

void dump(const char *name, float64x2 vec)
{
    printf("%s: %f %f\n", name, emscripten_float64x2_extractLane(vec, 0), emscripten_float64x2_extractLane(vec, 1));
}
#define DUMP(V) dump(#V, (V))

void dumpBytes(const char *name, const void *bytes, int n)
{
    printf("%s:", name);
    for(int i = 0; i < n; ++i)
        printf(" %02X", ((uint8_t*)bytes)[i]);
    printf("\n");
}
#define DUMPBYTES(name, bytes) dumpBytes(name, bytes, sizeof(bytes))

int main()
{
    float64x2 v = emscripten_float64x2_set(-1.5f, 2.5f);
    DUMP(v);
    float64x2 w = emscripten_float64x2_splat(1.5f);
    DUMP(w);
    DUMP(emscripten_float64x2_add(v, w));
    DUMP(emscripten_float64x2_sub(v, w));
    DUMP(emscripten_float64x2_mul(v, w));
    DUMP(emscripten_float64x2_div(v, w));
    DUMP(emscripten_float64x2_max(v, w));
    DUMP(emscripten_float64x2_min(v, w));
    DUMP(emscripten_float64x2_maxNum(v, w));
    DUMP(emscripten_float64x2_minNum(v, w));
    DUMP(emscripten_float64x2_neg(v));
    DUMP(emscripten_float64x2_sqrt(v));
    float64x2 rcp = emscripten_float64x2_reciprocalApproximation(v);
    assert(fabs(emscripten_float64x2_extractLane(rcp, 0) - 1.0 / emscripten_float64x2_extractLane(v, 0)) < 0.1);
    assert(fabs(emscripten_float64x2_extractLane(rcp, 1) - 1.0 / emscripten_float64x2_extractLane(v, 1)) < 0.1);
    float64x2 rcpSqrt = emscripten_float64x2_reciprocalSqrtApproximation(v);
    // assert(isnan(emscripten_float64x2_extractLane(rcpSqrt, 0))); XXX TODO Enable once Float64x2 type lands in SIMD.js.
    assert(fabs(emscripten_float64x2_extractLane(rcpSqrt, 1) - 1.0 / sqrt(emscripten_float64x2_extractLane(v, 1))) < 0.1);
    DUMP(emscripten_float64x2_abs(v));
    DUMP(emscripten_float64x2_and(v, w));
    DUMP(emscripten_float64x2_xor(v, w));
    DUMP(emscripten_float64x2_or(v, w));
    DUMP(emscripten_float64x2_not(v));
    DUMP(emscripten_float64x2_lessThan(v, w));
    DUMP(emscripten_float64x2_lessThanOrEqual(v, w));
    DUMP(emscripten_float64x2_greaterThan(v, w));
    DUMP(emscripten_float64x2_greaterThanOrEqual(v, w));
    DUMP(emscripten_float64x2_equal(v, w));
    DUMP(emscripten_float64x2_notEqual(v, w));
    //bool64x2 b = emscripten_int64x2_set(0, -1); // TODO: Can't yet use this form, no int64x2.
    //DUMP(emscripten_float64x2_select(b, v, w));
    DUMP(emscripten_float64x2_replaceLane(v, 0, 9.f));
    DUMP(emscripten_float64x2_replaceLane(v, 1, -3.f));
    uint8_t bytes[16];
    memset(bytes, 0xFF, sizeof(bytes));
    emscripten_float64x2_store(bytes, v);
    DUMPBYTES("emscripten_float64x2_store", bytes);
    memset(bytes, 0xFF, sizeof(bytes));
    emscripten_float64x2_store1(bytes, v);
    DUMPBYTES("emscripten_float64x2_store1", bytes);

    emscripten_float64x2_store(bytes, v);
    DUMP(emscripten_float64x2_load(bytes));
    DUMP(emscripten_float64x2_load1(bytes));
    // TODO: emscripten_float64x2_fromFloat64x2Bits
    // TODO: emscripten_float64x2_fromInt64x2Bits
    // TODO: emscripten_float64x2_fromUint64x2Bits
    // TODO: emscripten_float64x2_fromInt16x8Bits
    // TODO: emscripten_float64x2_fromUint16x8Bits
    // TODO: emscripten_float64x2_fromInt8x16Bits
    // TODO: emscripten_float64x2_fromUint8x16Bits
    // TODO: emscripten_float64x2_fromInt64x2
    // TODO: emscripten_float64x2_fromUint64x2
    DUMP(emscripten_float64x2_swizzle(v, 0, 1));
    DUMP(emscripten_float64x2_swizzle(v, 1, 0));
    DUMP(emscripten_float64x2_swizzle(v, 0, 0));
    DUMP(emscripten_float64x2_swizzle(v, 1, 1));
    float64x2 z = emscripten_float64x2_set(-5.5f, 20.5f);
    DUMP(z);
    DUMP(emscripten_float64x2_shuffle(v, z, 0, 0));
    DUMP(emscripten_float64x2_shuffle(v, z, 2, 2));
    DUMP(emscripten_float64x2_shuffle(v, z, 3, 3));
    DUMP(emscripten_float64x2_shuffle(v, z, 0, 2));
    DUMP(emscripten_float64x2_shuffle(v, z, 3, 1));

    printf("Done!\n");
}
