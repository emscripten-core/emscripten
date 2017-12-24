
var z = fleefl();
var zz = fleefl();
function g(a) {
 return a + 1;
}
Module["g"] = g;
function h(a) {
 return a + 1;
}
print(h(123));
((function() {
 var z = fleefl();
 var zz = fleefl();
 function g(a) {
  return a + 1;
 }
 Module["g"] = g;
 function hh(a) {
  return a + 1;
 }
 print(hh(123));
}))();
function glue() {
 function lookup() {
  throw 1;
 }
}
glue();
function _glCreateShader() {
 return 1;
}
function emulate() {
 _glCreateShader = function _glCreateShader(shaderType) {
  return glCreateShader();
 };
}
emulate();

