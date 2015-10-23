function badf() {
 var $9 = Math_fround(0);
 $9 = (HEAP32[tempDoublePtr>>2]=$8,Math_fround(HEAPF32[tempDoublePtr>>2]));
 HEAPF32[$gep23_asptr>>2] = $9;
}
function badf2() {
 var $9 = 0;
 $9 = (HEAPF32[tempDoublePtr>>2]=$8,HEAP32[tempDoublePtr>>2]|0);
 HEAP32[$gep23_asptr>>2] = $9;
}
function dupe() {
 x = Math_fround(x);
 x = Math_fround(Math_fround(x));
 x = Math_fround(Math_fround(Math_fround(x)));
 x = Math_fround(Math_fround(Math_fround(Math_fround(x))));
}
function zeros(x) {
  x = Math_fround(x);
  var y = Math_fround(0);
  print(Math_fround(y) + Math_fround(0));
  return Math_fround(0); // return needs to stay as is
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["badf", "badf2", "dupe", "zeros"]
