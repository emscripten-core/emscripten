function test() {
 var f1 = f0;
 if (HEAPF32[(0 | 0) >> 2] <= HEAPF32[(1 | 0) >> 2]) {
  f1 = Math_fround(+1);
  return Math_fround(f1);
 } else {
  f1 = Math_fround(+2);
  return Math_fround(f1);
 }
 return f0;
}
