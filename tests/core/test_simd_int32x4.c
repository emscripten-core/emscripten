#include <emscripten/vector.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

void dump(const char *name, int32x4 vec)
{
    printf("%s: %d %d %d %d\n", name, emscripten_int32x4_extractLane(vec, 0), emscripten_int32x4_extractLane(vec, 1), emscripten_int32x4_extractLane(vec, 2), emscripten_int32x4_extractLane(vec, 3));
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
    int32x4 v = emscripten_int32x4_set(1, 0, 1, 4);
    DUMP(v);
    int32x4 w = emscripten_int32x4_splat(2);
    DUMP(w);
    DUMP(emscripten_int32x4_add(v, w));
    DUMP(emscripten_int32x4_sub(v, w));
    DUMP(emscripten_int32x4_mul(v, w));
    DUMP(emscripten_int32x4_neg(v));
    DUMP(emscripten_int32x4_and(v, w));
    DUMP(emscripten_int32x4_xor(v, w));
    DUMP(emscripten_int32x4_or(v, w));
    DUMP(emscripten_int32x4_not(v));
    DUMP(emscripten_int32x4_lessThan(v, w));
    DUMP(emscripten_int32x4_lessThanOrEqual(v, w));
    DUMP(emscripten_int32x4_greaterThan(v, w));
    DUMP(emscripten_int32x4_greaterThanOrEqual(v, w));
    DUMP(emscripten_int32x4_equal(v, w));
    DUMP(emscripten_int32x4_notEqual(v, w));
//    DUMPINT(emscripten_int32x4_anyTrue(v)); // XXX TODO: Figure out if there is a no-op cast from int32x4->bool32x4 and enable this, or remove if this doesn't make sense.
//    DUMPINT(emscripten_int32x4_allTrue(v)); // XXX TODO: Figure out if there is a no-op cast from int32x4->bool32x4 and enable this, or remove if this doesn't make sense.
    DUMP(emscripten_int32x4_shiftLeftByScalar(v, 0));
    DUMP(emscripten_int32x4_shiftLeftByScalar(v, 1));
    DUMP(emscripten_int32x4_shiftLeftByScalar(v, 2));
    DUMP(emscripten_int32x4_shiftLeftByScalar(v, 16));
    DUMP(emscripten_int32x4_shiftLeftByScalar(v, 32));
    DUMP(emscripten_int32x4_shiftLeftByScalar(v, 48));
    DUMP(emscripten_int32x4_shiftRightByScalar(v, 0));
    DUMP(emscripten_int32x4_shiftRightByScalar(v, 1));
    DUMP(emscripten_int32x4_shiftRightByScalar(v, 2));
    DUMP(emscripten_int32x4_shiftRightByScalar(v, 16));
    DUMP(emscripten_int32x4_shiftRightByScalar(v, 32));
    DUMP(emscripten_int32x4_shiftRightByScalar(v, 48));
    bool32x4 b = emscripten_int32x4_set(0, -1, 0, -1);
    DUMP(emscripten_int32x4_select(b, v, w));
    DUMP(emscripten_int32x4_replaceLane(v, 0, 9));
    DUMP(emscripten_int32x4_replaceLane(v, 1, 3));
    DUMP(emscripten_int32x4_replaceLane(v, 2, 0));
    DUMP(emscripten_int32x4_replaceLane(v, 3, 91));
    uint8_t bytes[16];
    memset(bytes, 0xFF, sizeof(bytes));
    emscripten_int32x4_store(bytes, v);
    DUMPBYTES("emscripten_int32x4_store", bytes);
    memset(bytes, 0xFF, sizeof(bytes));
    emscripten_int32x4_store1(bytes, v);
    DUMPBYTES("emscripten_int32x4_store1", bytes);
    memset(bytes, 0xFF, sizeof(bytes));
    emscripten_int32x4_store2(bytes, v);
    DUMPBYTES("emscripten_int32x4_store2", bytes);
    memset(bytes, 0xFF, sizeof(bytes));
    emscripten_int32x4_store3(bytes, v);
    DUMPBYTES("emscripten_int32x4_store3", bytes);

    emscripten_int32x4_store(bytes, v);
    DUMP(emscripten_int32x4_load(bytes));
    DUMP(emscripten_int32x4_load1(bytes));
    DUMP(emscripten_int32x4_load2(bytes));
    DUMP(emscripten_int32x4_load3(bytes));
    // TODO: emscripten_int32x4_fromFloat64x2Bits
    // TODO: emscripten_int32x4_fromInt32x4Bits
    // TODO: emscripten_int32x4_fromUint32x4Bits
    // TODO: emscripten_int32x4_fromInt16x8Bits
    // TODO: emscripten_int32x4_fromUint16x8Bits
    // TODO: emscripten_int32x4_fromInt8x16Bits
    // TODO: emscripten_int32x4_fromUint8x16Bits
    // TODO: emscripten_int32x4_fromInt32x4
    // TODO: emscripten_int32x4_fromUint32x4
    DUMP(emscripten_int32x4_swizzle(v, 0, 1, 2, 3));
    DUMP(emscripten_int32x4_swizzle(v, 3, 2, 1, 0));
    DUMP(emscripten_int32x4_swizzle(v, 0, 0, 0, 0));
    DUMP(emscripten_int32x4_swizzle(v, 0, 3, 0, 3));
    DUMP(emscripten_int32x4_swizzle(v, 3, 3, 3, 3));
    int32x4 z = emscripten_int32x4_set(-5, 20, 14, 9);
    DUMP(z);
    DUMP(emscripten_int32x4_shuffle(v, z, 0, 0, 0, 0));
    DUMP(emscripten_int32x4_shuffle(v, z, 4, 4, 4, 4));
    DUMP(emscripten_int32x4_shuffle(v, z, 7, 7, 7, 7));
    DUMP(emscripten_int32x4_shuffle(v, z, 0, 2, 4, 6));
    DUMP(emscripten_int32x4_shuffle(v, z, 7, 0, 3, 5));
    printf("Done!\n");
}
