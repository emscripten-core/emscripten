function f() {
  var $2 = 0;
  (growMemViews(), HEAP32)[$0 >> 2] = $2 + 1;
  $9 = (growMemViews(), HEAPU8)[$2 >> 0] | 0;
  +(growMemViews(), HEAPF64)[x >> 3];
  (growMemViews(), HEAP64)[x >> 3] = (growMemViews(), HEAP64)[y >> 3];
}

function libraryFunc(ptr, val) {
  if (ptr < (growMemViews(), HEAP8).length) {
    Atomics.wait((growMemViews(), HEAP32), ptr, val);
  }
}
