function a() {
  f((HEAPU8[10202] | 0) + 5 | 0);
  f((HEAPU8[10202] | 0) | 0);
  f(347 | 0);
  f(347 | 12);
  f(347 & 12);
  HEAP[4096 >> 2] = 5;
  HEAP[(4096 & 8191) >> 2] = 5;
  whee(12, 13) | 0;
  +whee(12, 13);
}
function b($this, $__n) {
  $this = $this | 0;
  $__n = $__n | 0;
  var $4 = 0, $5 = 0, $10 = 0, $13 = 0, $14 = 0, $15 = 0, $23 = 0, $30 = 0, $38 = 0, $40 = 0;
  if (($__n | 0) == 0) {
    return;
  }
  $4 = $this;
  $5 = HEAP8[$4 & 16777215] | 0;
  if (($5 & 1) << 24 >> 24 == 0) {
    $14 = 10;
    $13 = $5;
  } else {
    $10 = HEAP32[(($this | 0) & 16777215) >> 2] | 0;
    $14 = ($10 & -2) - 1 | 0;
    $13 = $10 & 255;
  }
  $15 = $13 & 255;
  if (($15 & 1 | 0) == 0) {
    $23 = $15 >>> 1;
  } else {
    $23 = HEAP32[(($this + 4 | 0) & 16777215) >> 2] | 0;
  }
  if (($14 - $23 | 0) >>> 0 < $__n >>> 0) {
    __ZNSt3__112basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEE9__grow_byEjjjjjj($this, $14, ($__n - $14 | 0) + $23 | 0, $23, $23);
    $30 = HEAP8[$4 & 16777215] | 0;
  } else {
    $30 = $13;
  }
  if (($30 & 1) << 24 >> 24 == 0) {
    $38 = $this + 1 | 0;
  } else {
    $38 = HEAP32[(($this + 8 | 0) & 16777215) >> 2] | 0;
  }
  _memset($38 + $23 | 0 | 0 | 0, 0 | 0 | 0, $__n | 0 | 0, 1 | 0 | 0, 1213141516);
  $40 = $23 + $__n | 0;
  if (((HEAP8[$4 & 16777215] | 0) & 1) << 24 >> 24 == 0) {
    HEAP8[$4 & 16777215] = $40 << 1 & 255;
  } else {
    HEAP32[(($this + 4 | 0) & 16777215) >> 2] = $40;
  }
  HEAP8[($38 + $40 | 0) & 16777215] = 0;
  return;
}
function rett() {
  if (f()) {
    g();
    return 5;
  }
  // missing final return, need to add it
}
function ret2t() {
  if (f()) {
    g();
    return;
  }
  // missing final return, but no need
}
function retf() {
  if (f()) {
    g();
    return +h();
  }
  // missing final return, need it as a float
}
function i32_8() {
  if (((HEAP8[$4 & 16777215] | 0) << 24 >> 24) == 0) {
    print(5);
  }
  if ((HEAP8[$5 & 16777215] << 24 >> 24) == 0) {
    print(5);
  }
  if (((HEAPU8[$6 & 16777215] | 0) << 24 >> 24) == 0) {
    print(5);
  }
  if ((HEAPU8[$7 & 16777215] << 24 >> 24) == 0) {
    print(5);
  }
  // non-valid
  if ((HEAPU8[$8 & 16777215] << 24 >> 16) == 0) {
    print(5);
  }
  if ((HEAPU8[$9 & 16777215] << 16 >> 16) == 0) {
    print(5);
  }
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["a", "b", "rett", "ret2t", "retf", "i32_8"]
