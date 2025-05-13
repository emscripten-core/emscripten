_asan_js_store(HEAP8, x, 1);

_asan_js_store(HEAP16, x, 2);

_asan_js_store(HEAP32, x, 3);

_asan_js_store(HEAPU8, x, 4);

_asan_js_store(HEAPU16, x, 5);

_asan_js_store(HEAPU32, x, 6);

_asan_js_store(HEAPF32, x, 7);

_asan_js_store(HEAPF64, x, 8);

_asan_js_store(HEAP64, x, 9n);

_asan_js_store(HEAPU64, x, 10n);

a1 = _asan_js_load(HEAP8, x);

a2 = _asan_js_load(HEAP16, x);

a3 = _asan_js_load(HEAP32, x);

a4 = _asan_js_load(HEAPU8, x);

a5 = _asan_js_load(HEAPU16, x);

a6 = _asan_js_load(HEAPU32, x);

a7 = _asan_js_load(HEAPF32, x);

a8 = _asan_js_load(HEAPF64, x);

a9 = _asan_js_load(HEAP64, x);

a10 = _asan_js_load(HEAPU64, x);

foo = _asan_js_store(HEAPU8, 1337, 42);

_asan_js_load(HEAP16, bar(_asan_js_load(HEAPF64, 5)));

_asan_js_store(HEAPF32, x, _asan_js_load(HEAP32, y));

function _asan_js_load(ptr) {
  return HEAP8[ptr];
}

function somethingElse() {
  return _asan_js_load(HEAP8, ptr);
}

HEAP8.length;

_asan_js_load(HEAP8, length);
