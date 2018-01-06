
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
for (var cant_remove_me_i_am_in_a_for_in in new_node.contents) {}
for (var cant_remove_me_i_am_in_a_for = 0; cant_remove_me_i_am_in_a_for < 10; cant_remove_me_i_am_in_a_for++) {}



