function f() {
  var $2 = 0;
  GROWABLE_HEAP_I32()[$0 >> 2] = $2 + 1;
  $9 = GROWABLE_HEAP_U8()[$2 >> 0] | 0;
  +GROWABLE_HEAP_F64()[x >> 3];
}

function libraryFunc(ptr, val) {
  if (ptr < GROWABLE_HEAP_I8().length) {
    Atomics.wait(GROWABLE_HEAP_I32(), ptr, val);
  }
}
