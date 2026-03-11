HEAP8[_asan_js_check_index(HEAP8, x, ___asan_storeN)] = 1;

HEAP16[_asan_js_check_index(HEAP16, x, ___asan_storeN)] = 2;

HEAP32[_asan_js_check_index(HEAP32, x, ___asan_storeN)] = 3;

HEAPU8[_asan_js_check_index(HEAPU8, x, ___asan_storeN)] = 4;

HEAPU16[_asan_js_check_index(HEAPU16, x, ___asan_storeN)] = 5;

HEAPU32[_asan_js_check_index(HEAPU32, x, ___asan_storeN)] = 6;

HEAPF32[_asan_js_check_index(HEAPF32, x, ___asan_storeN)] = 7;

HEAPF64[_asan_js_check_index(HEAPF64, x, ___asan_storeN)] = 8;

HEAP64[_asan_js_check_index(HEAP64, x, ___asan_storeN)] = 9n;

HEAPU64[_asan_js_check_index(HEAPU64, x, ___asan_storeN)] = 10n;

a1 = HEAP8[_asan_js_check_index(HEAP8, x, ___asan_loadN)];

a2 = HEAP16[_asan_js_check_index(HEAP16, x, ___asan_loadN)];

a3 = HEAP32[_asan_js_check_index(HEAP32, x, ___asan_loadN)];

a4 = HEAPU8[_asan_js_check_index(HEAPU8, x, ___asan_loadN)];

a5 = HEAPU16[_asan_js_check_index(HEAPU16, x, ___asan_loadN)];

a6 = HEAPU32[_asan_js_check_index(HEAPU32, x, ___asan_loadN)];

a7 = HEAPF32[_asan_js_check_index(HEAPF32, x, ___asan_loadN)];

a8 = HEAPF64[_asan_js_check_index(HEAPF64, x, ___asan_loadN)];

a9 = HEAP64[_asan_js_check_index(HEAP64, x, ___asan_loadN)];

a10 = HEAPU64[_asan_js_check_index(HEAPU64, x, ___asan_loadN)];

foo = HEAPU8[_asan_js_check_index(HEAPU8, 1337, ___asan_storeN)] = 42;

HEAP16[_asan_js_check_index(HEAP16, bar(HEAPF64[_asan_js_check_index(HEAPF64, 5, ___asan_loadN)]), ___asan_loadN)];

HEAPF32[_asan_js_check_index(HEAPF32, x, ___asan_storeN)] = HEAP32[_asan_js_check_index(HEAP32, y, ___asan_loadN)];

function establishStackSpace() {
  HEAP32[0];
}

function somethingElse() {
  return HEAP8[_asan_js_check_index(HEAP8, ptr, ___asan_loadN)];
}

HEAP8.length;

HEAP8[_asan_js_check_index(HEAP8, length, ___asan_loadN)];
