#include <emscripten/vector.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

void dump(const char *name, int8x16 vec)
{
    printf("%s: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", name, emscripten_int8x16_extractLane(vec, 0), emscripten_int8x16_extractLane(vec, 1), emscripten_int8x16_extractLane(vec, 2), emscripten_int8x16_extractLane(vec, 3),
        emscripten_int8x16_extractLane(vec, 4), emscripten_int8x16_extractLane(vec, 5), emscripten_int8x16_extractLane(vec, 6), emscripten_int8x16_extractLane(vec, 7),
        emscripten_int8x16_extractLane(vec, 8), emscripten_int8x16_extractLane(vec, 9), emscripten_int8x16_extractLane(vec, 10), emscripten_int8x16_extractLane(vec, 11),
        emscripten_int8x16_extractLane(vec, 12), emscripten_int8x16_extractLane(vec, 13), emscripten_int8x16_extractLane(vec, 14), emscripten_int8x16_extractLane(vec, 15));
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
    int8x16 v = emscripten_int8x16_set(1, 0, 1, 127, -2, 9, 13, -128, 3, 19, 73, -53, 42, 66, 10, 100);
    DUMP(v);
    int8x16 w = emscripten_int8x16_splat(2);
    DUMP(w);
    DUMP(emscripten_int8x16_add(v, w));
    DUMP(emscripten_int8x16_sub(v, w));
    DUMP(emscripten_int8x16_mul(v, w));
    DUMP(emscripten_int8x16_neg(v));
    DUMP(emscripten_int8x16_and(v, w));
    DUMP(emscripten_int8x16_xor(v, w));
    DUMP(emscripten_int8x16_or(v, w));
    DUMP(emscripten_int8x16_not(v));
    DUMP(emscripten_int8x16_lessThan(v, w));
    DUMP(emscripten_int8x16_lessThanOrEqual(v, w));
    DUMP(emscripten_int8x16_greaterThan(v, w));
    DUMP(emscripten_int8x16_greaterThanOrEqual(v, w));
    DUMP(emscripten_int8x16_equal(v, w));
    DUMP(emscripten_int8x16_notEqual(v, w));
//    DUMPINT(emscripten_int8x16_anyTrue(v)); // XXX TODO: Figure out if there is a no-op cast from int8x16->bool8x16 and enable this, or remove if this doesn't make sense.
//    DUMPINT(emscripten_int8x16_allTrue(v)); // XXX TODO: Figure out if there is a no-op cast from int8x16->bool8x16 and enable this, or remove if this doesn't make sense.
    DUMP(emscripten_int8x16_shiftLeftByScalar(v, 0));
    DUMP(emscripten_int8x16_shiftLeftByScalar(v, 1));
    DUMP(emscripten_int8x16_shiftLeftByScalar(v, 2));
    DUMP(emscripten_int8x16_shiftLeftByScalar(v, 16));
    DUMP(emscripten_int8x16_shiftLeftByScalar(v, 32));
    DUMP(emscripten_int8x16_shiftLeftByScalar(v, 48));
    DUMP(emscripten_int8x16_shiftRightByScalar(v, 0));
    DUMP(emscripten_int8x16_shiftRightByScalar(v, 1));
    DUMP(emscripten_int8x16_shiftRightByScalar(v, 2));
    DUMP(emscripten_int8x16_shiftRightByScalar(v, 16));
    DUMP(emscripten_int8x16_shiftRightByScalar(v, 32));
    DUMP(emscripten_int8x16_shiftRightByScalar(v, 48));
    bool32x4 b = emscripten_int8x16_set(0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1, 0, -1);
    DUMP(emscripten_int8x16_select(b, v, w));
    DUMP(emscripten_int8x16_addSaturate(v, w));
    DUMP(emscripten_int8x16_subSaturate(v, w));
    DUMP(emscripten_int8x16_replaceLane(v, 0, 9));
    DUMP(emscripten_int8x16_replaceLane(v, 1, 3));
    DUMP(emscripten_int8x16_replaceLane(v, 2, 0));
    DUMP(emscripten_int8x16_replaceLane(v, 3, 91));
    DUMP(emscripten_int8x16_replaceLane(v, 4, 127));
    DUMP(emscripten_int8x16_replaceLane(v, 5, 100));
    DUMP(emscripten_int8x16_replaceLane(v, 6, -100));
    DUMP(emscripten_int8x16_replaceLane(v, 7, -128));
    DUMP(emscripten_int8x16_replaceLane(v, 8, 9));
    DUMP(emscripten_int8x16_replaceLane(v, 9, 3));
    DUMP(emscripten_int8x16_replaceLane(v, 10, 0));
    DUMP(emscripten_int8x16_replaceLane(v, 11, 91));
    DUMP(emscripten_int8x16_replaceLane(v, 12, 127));
    DUMP(emscripten_int8x16_replaceLane(v, 13, 100));
    DUMP(emscripten_int8x16_replaceLane(v, 14, -100));
    DUMP(emscripten_int8x16_replaceLane(v, 15, -128));
    uint8_t bytes[16];
    memset(bytes, 0xFF, sizeof(bytes));
    emscripten_int8x16_store(bytes, v);
    DUMPBYTES("emscripten_int8x16_store", bytes);
    DUMP(emscripten_int8x16_load(bytes));
    // TODO: emscripten_int8x16_fromFloat64x2Bits
    // TODO: emscripten_int8x16_fromint8x16Bits
    // TODO: emscripten_int8x16_fromUint8x16Bits
    // TODO: emscripten_int8x16_fromInt8x16Bits
    // TODO: emscripten_int8x16_fromUint8x16Bits
    // TODO: emscripten_int8x16_fromInt8x16Bits
    // TODO: emscripten_int8x16_fromUint8x16Bits
    // TODO: emscripten_int8x16_fromint8x16
    // TODO: emscripten_int8x16_fromUint8x16
    DUMP(emscripten_int8x16_swizzle(v, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15));
    DUMP(emscripten_int8x16_swizzle(v, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0));
    DUMP(emscripten_int8x16_swizzle(v, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
    DUMP(emscripten_int8x16_swizzle(v, 0, 3, 0, 3, 7, 1, 2, 6, 9, 14, 5, 12, 15, 4, 10, 2));
    DUMP(emscripten_int8x16_swizzle(v, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3));
    int8x16 z = emscripten_int8x16_set(-5, 20, 14, 9, 0, 11, 120, -120, 40, 50, 60, 70, 80, 90, 100, 110);
    DUMP(z);
    DUMP(emscripten_int8x16_shuffle(v, z, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0));
    DUMP(emscripten_int8x16_shuffle(v, z, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4));
    DUMP(emscripten_int8x16_shuffle(v, z, 15, 15, 15, 15, 7, 7, 7, 7, 31, 31, 31, 31, 15, 14, 13, 12));
    DUMP(emscripten_int8x16_shuffle(v, z, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30));
    DUMP(emscripten_int8x16_shuffle(v, z, 7, 0, 3, 5, 9, 11, 1, 4, 6, 12, 19, 4, 9, 0, 2, 3));
    printf("Done!\n");
}
