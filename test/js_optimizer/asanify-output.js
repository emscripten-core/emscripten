_asan_js_store_1(x, 1);

_asan_js_store_2(x, 2);

_asan_js_store_4(x, 3);

_asan_js_store_1u(x, 4);

_asan_js_store_2u(x, 5);

_asan_js_store_4u(x, 6);

_asan_js_store_f(x, 7);

_asan_js_store_d(x, 8);

a1 = _asan_js_load_1(x);

a2 = _asan_js_load_2(x);

a3 = _asan_js_load_4(x);

a4 = _asan_js_load_1u(x);

a5 = _asan_js_load_2u(x);

a6 = _asan_js_load_4u(x);

a7 = _asan_js_load_f(x);

a8 = _asan_js_load_d(x);

foo = _asan_js_store_1u(1337, 42);

_asan_js_load_2(bar(_asan_js_load_d(5)));

_asan_js_store_f(x, _asan_js_load_4(y));

function _asan_js_load_1(ptr) {
  return HEAP8[ptr];
}

function somethingElse() {
  return _asan_js_load_1(ptr);
}

HEAP8.length;

_asan_js_load_1(length);
