HEAP8[SAFE_HEAP_INDEX(HEAP8, x, "storing")] = 1;

HEAP16[SAFE_HEAP_INDEX(HEAP16, x, "storing")] = 2;

HEAP32[SAFE_HEAP_INDEX(HEAP32, x, "storing")] = 3;

HEAPU8[SAFE_HEAP_INDEX(HEAPU8, x, "storing")] = 4;

HEAPU16[SAFE_HEAP_INDEX(HEAPU16, x, "storing")] = 5;

HEAPU32[SAFE_HEAP_INDEX(HEAPU32, x, "storing")] = 6;

HEAPF32[SAFE_HEAP_INDEX(HEAPF32, x, "storing")] = 7;

HEAPF64[SAFE_HEAP_INDEX(HEAPF64, x, "storing")] = 8;

HEAP64[SAFE_HEAP_INDEX(HEAP64, x, "storing")] = 9n;

HEAPU64[SAFE_HEAP_INDEX(HEAPU64, x, "storing")] = 10n;

a1 = HEAP8[SAFE_HEAP_INDEX(HEAP8, x, "loading")];

a2 = HEAP16[SAFE_HEAP_INDEX(HEAP16, x, "loading")];

a3 = HEAP32[SAFE_HEAP_INDEX(HEAP32, x, "loading")];

a4 = HEAPU8[SAFE_HEAP_INDEX(HEAPU8, x, "loading")];

a5 = HEAPU16[SAFE_HEAP_INDEX(HEAPU16, x, "loading")];

a6 = HEAPU32[SAFE_HEAP_INDEX(HEAPU32, x, "loading")];

a7 = HEAPF32[SAFE_HEAP_INDEX(HEAPF32, x, "loading")];

a8 = HEAPF64[SAFE_HEAP_INDEX(HEAPF64, x, "loading")];

a9 = HEAP64[SAFE_HEAP_INDEX(HEAP64, x, "loading")];

a10 = HEAPU64[SAFE_HEAP_INDEX(HEAPU64, x, "loading")];

foo = HEAPU8[SAFE_HEAP_INDEX(HEAPU8, 1337, "storing")] = 42;

HEAP16[SAFE_HEAP_INDEX(HEAP16, bar(HEAPF64[SAFE_HEAP_INDEX(HEAPF64, 5, "loading")]), "loading")];

HEAPF32[SAFE_HEAP_INDEX(HEAPF32, x, "storing")] = HEAP32[SAFE_HEAP_INDEX(HEAP32, y, "loading")];

function somethingElse() {
  return HEAP8[SAFE_HEAP_INDEX(HEAP8, ptr, "loading")];
}

HEAP8.length;

HEAP8[SAFE_HEAP_INDEX(HEAP8, length, "loading")];
