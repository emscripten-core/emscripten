// EMSCRIPTEN_START_ASM
var asm = (function(global, env, buffer) {
 "use asm";
// EMSCRIPTEN_START_FUNCS
 function a() {
  return 0;
 }
 function b() {
  return 0;
 }
 function c() {
  a();
  return;
 }
 function d() {
  b();
  e();
  return;
 }
// EMSCRIPTEN_END_FUNCS
  var f = [ b ];
})(Module.asmGlobalArg, Module.asmLibraryArg, buffer);
// EMSCRIPTEN_END_ASM