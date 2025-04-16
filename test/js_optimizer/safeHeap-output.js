SAFE_HEAP_STORE(x, 1, 1);

SAFE_HEAP_STORE(x * 2, 2, 2);

SAFE_HEAP_STORE(x * 4, 3, 4);

SAFE_HEAP_STORE(x, 4, 1);

SAFE_HEAP_STORE(x * 2, 5, 2);

SAFE_HEAP_STORE(x * 4, 6, 4);

SAFE_HEAP_STORE_D(x * 4, 7, 4);

SAFE_HEAP_STORE_D(x * 8, 8, 8);

a1 = SAFE_HEAP_LOAD(x, 1, 0);

a2 = SAFE_HEAP_LOAD(x * 2, 2, 0);

a3 = SAFE_HEAP_LOAD(x * 4, 4, 0);

a4 = SAFE_HEAP_LOAD(x, 1, 1);

a5 = SAFE_HEAP_LOAD(x * 2, 2, 1);

a6 = SAFE_HEAP_LOAD(x * 4, 4, 1);

a7 = SAFE_HEAP_LOAD_D(x * 4, 4, 0);

a8 = SAFE_HEAP_LOAD_D(x * 8, 8, 0);

foo = SAFE_HEAP_STORE(1337, 42, 1);

SAFE_HEAP_LOAD(bar(SAFE_HEAP_LOAD_D(5 * 8, 8, 0)) * 2, 2, 0);

SAFE_HEAP_STORE_D(x * 4, SAFE_HEAP_LOAD(y * 4, 4, 0), 4);

function SAFE_HEAP_FOO(ptr) {
  return HEAP8[ptr];
}

function setValue_safe(ptr) {
  return HEAP8[ptr];
}

function getValue_safe(ptr) {
  return HEAP8[ptr];
}

function somethingElse() {
  return SAFE_HEAP_LOAD(ptr, 1, 0);
}

HEAP8.length;

SAFE_HEAP_LOAD(length, 1, 0);
