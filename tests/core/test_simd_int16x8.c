#include <emscripten/vector.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

void dump(const char *name, int16x8 vec)
{
    printf("%s: %d %d %d %d %d %d %d %d\n", name, emscripten_int16x8_extractLane(vec, 0), emscripten_int16x8_extractLane(vec, 1), emscripten_int16x8_extractLane(vec, 2), emscripten_int16x8_extractLane(vec, 3),
        emscripten_int16x8_extractLane(vec, 4), emscripten_int16x8_extractLane(vec, 5), emscripten_int16x8_extractLane(vec, 6), emscripten_int16x8_extractLane(vec, 7));
}
#define DUMP(V) dump(#V, (V))
#define DUMPINT(V) printf("%s: %d\n", #V, V)

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
    int16x8 v = emscripten_int16x8_set(1, 0, 1, 32767, -2, 9, 13, -32768);
    DUMP(v);
    int16x8 w = emscripten_int16x8_splat(2);
    DUMP(w);
    DUMP(emscripten_int16x8_add(v, w));
    DUMP(emscripten_int16x8_sub(v, w));
    DUMP(emscripten_int16x8_mul(v, w));
    DUMP(emscripten_int16x8_neg(v));
    DUMP(emscripten_int16x8_and(v, w));
    DUMP(emscripten_int16x8_xor(v, w));
    DUMP(emscripten_int16x8_or(v, w));
    DUMP(emscripten_int16x8_not(v));
    DUMP(emscripten_int16x8_lessThan(v, w));
    DUMP(emscripten_int16x8_lessThanOrEqual(v, w));
    DUMP(emscripten_int16x8_greaterThan(v, w));
    DUMP(emscripten_int16x8_greaterThanOrEqual(v, w));
    DUMP(emscripten_int16x8_equal(v, w));
    DUMP(emscripten_int16x8_notEqual(v, w));
//    DUMPINT(emscripten_int16x8_anyTrue(v)); // XXX TODO: Figure out if there is a no-op cast from int16x8->bool16x8 and enable this, or remove if this doesn't make sense.
//    DUMPINT(emscripten_int16x8_allTrue(v)); // XXX TODO: Figure out if there is a no-op cast from int16x8->bool16x8 and enable this, or remove if this doesn't make sense.
    DUMP(emscripten_int16x8_shiftLeftByScalar(v, 0));
    DUMP(emscripten_int16x8_shiftLeftByScalar(v, 1));
    DUMP(emscripten_int16x8_shiftLeftByScalar(v, 2));
    DUMP(emscripten_int16x8_shiftLeftByScalar(v, 16));
    DUMP(emscripten_int16x8_shiftLeftByScalar(v, 32));
    DUMP(emscripten_int16x8_shiftLeftByScalar(v, 48));
    DUMP(emscripten_int16x8_shiftRightByScalar(v, 0));
    DUMP(emscripten_int16x8_shiftRightByScalar(v, 1));
    DUMP(emscripten_int16x8_shiftRightByScalar(v, 2));
    DUMP(emscripten_int16x8_shiftRightByScalar(v, 16));
    DUMP(emscripten_int16x8_shiftRightByScalar(v, 32));
    DUMP(emscripten_int16x8_shiftRightByScalar(v, 48));
    bool32x4 b = emscripten_int16x8_set(0, -1, 0, -1, 0, -1, 0, -1);
    DUMP(emscripten_int16x8_select(b, v, w));
    DUMP(emscripten_int16x8_addSaturate(v, w));
    DUMP(emscripten_int16x8_subSaturate(v, w));
    DUMP(emscripten_int16x8_replaceLane(v, 0, 9));
    DUMP(emscripten_int16x8_replaceLane(v, 1, 3));
    DUMP(emscripten_int16x8_replaceLane(v, 2, 0));
    DUMP(emscripten_int16x8_replaceLane(v, 3, 91));
    DUMP(emscripten_int16x8_replaceLane(v, 4, 32767));
    DUMP(emscripten_int16x8_replaceLane(v, 5, 100));
    DUMP(emscripten_int16x8_replaceLane(v, 6, -100));
    DUMP(emscripten_int16x8_replaceLane(v, 7, -32768));
    uint8_t bytes[16];
    memset(bytes, 0xFF, sizeof(bytes));
    emscripten_int16x8_store(bytes, v);
    DUMPBYTES("emscripten_int16x8_store", bytes);
    DUMP(emscripten_int16x8_load(bytes));
    // TODO: emscripten_int16x8_fromFloat64x2Bits
    // TODO: emscripten_int16x8_fromint16x8Bits
    // TODO: emscripten_int16x8_fromUint16x8Bits
    // TODO: emscripten_int16x8_fromInt16x8Bits
    // TODO: emscripten_int16x8_fromUint16x8Bits
    // TODO: emscripten_int16x8_fromInt8x16Bits
    // TODO: emscripten_int16x8_fromUint8x16Bits
    // TODO: emscripten_int16x8_fromint16x8
    // TODO: emscripten_int16x8_fromUint16x8
    DUMP(emscripten_int16x8_swizzle(v, 0, 1, 2, 3, 4, 5, 6, 7));
    DUMP(emscripten_int16x8_swizzle(v, 7, 6, 5, 4, 3, 2, 1, 0));
    DUMP(emscripten_int16x8_swizzle(v, 0, 0, 0, 0, 0, 0, 0, 0));
    DUMP(emscripten_int16x8_swizzle(v, 0, 3, 0, 3, 7, 1, 2, 6));
    DUMP(emscripten_int16x8_swizzle(v, 3, 3, 3, 3, 3, 3, 3, 3));
    int16x8 z = emscripten_int16x8_set(-5, 20, 14, 9, 0, 11, 32764, -32750);
    DUMP(z);
    DUMP(emscripten_int16x8_shuffle(v, z, 0, 0, 0, 0, 0, 0, 0, 0));
    DUMP(emscripten_int16x8_shuffle(v, z, 4, 4, 4, 4, 4, 4, 4, 4));
    DUMP(emscripten_int16x8_shuffle(v, z, 15, 15, 15, 15, 7, 7, 7, 7));
    DUMP(emscripten_int16x8_shuffle(v, z, 0, 2, 4, 6, 8, 10, 12, 14));
    DUMP(emscripten_int16x8_shuffle(v, z, 7, 0, 3, 5, 9, 11, 1, 4));
    printf("Done!\n");
}
