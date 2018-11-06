function test() {
 var $x = Math_fround(0);
 if (HEAPF32[(0|0)>>2] <= HEAPF32[(1|0)>>2]) {
  $x = Math_fround(+1);
  return (Math_fround($x));
 } else {
  $x = Math_fround(+2);
  return (Math_fround($x));
 }
 return Math_fround((Math_fround(+0)));
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["test"]
