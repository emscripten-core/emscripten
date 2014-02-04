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
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["a", "b", "rett", "ret2t", "retf", "i32_8", "tempDoublePtr", "boxx", "_main", "badf", "badf2"]
