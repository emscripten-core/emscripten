function ors($a,$b,$c,$d) {
 if ((((($a|0)==0) | (($b|0)==0)) | (($c|0)==0)) | (($d|0)==0)) {
  foo();
 }
}
function ands($a,$b,$c,$d) {
 if ((((($a|0)==0) & (($b|0)==0)) & (($c|0)==0)) & (($d|0)==0)) {
  foo();
 }
}
function not_ors($a,$b,$c,$d) {
 if (!((((($a|0)==0) | (($b|0)==0)) | (($c|0)==0)) | (($d|0)==0))) {
  foo();
 }
}
function not_ands($a,$b,$c,$d) {
 if (!((((($a|0)==0) & (($b|0)==0)) & (($c|0)==0)) & (($d|0)==0))) {
  foo();
 }
}
// EMSCRIPTEN_GENERATED_FUNCTIONS: ["ors", "ands", "not_ors", "not_ands"]  
