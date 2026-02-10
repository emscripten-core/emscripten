// stores
HEAP8[x] = 1;
HEAP16[x] = 2;
HEAP32[x] = 3;
HEAPU8[x] = 4;
HEAPU16[x] = 5;
HEAPU32[x] = 6;
HEAPF32[x] = 7;
HEAPF64[x] = 8;
HEAP64[x] = 9n;
HEAPU64[x] = 10n;

// loads
a1 = HEAP8[x];
a2 = HEAP16[x];
a3 = HEAP32[x];
a4 = HEAPU8[x];
a5 = HEAPU16[x];
a6 = HEAPU32[x];
a7 = HEAPF32[x];
a8 = HEAPF64[x];
a9 = HEAP64[x];
a10 = HEAPU64[x];

// store return value
foo = HEAPU8[1337] = 42;

// nesting
HEAP16[bar(HEAPF64[5])];
HEAPF32[x] = HEAP32[y];

// skip establishStackSpace, because it sets up variables used by ASan itself
function establishStackSpace() {
  HEAP32[0];
}

// but do handle everything else
function somethingElse() {
  return HEAP8[ptr];
}

// ignore a.X
HEAP8.length;
HEAP8[length];
