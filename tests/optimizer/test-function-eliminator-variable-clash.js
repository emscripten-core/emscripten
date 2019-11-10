var asm = (function(global, env, buffer) {
 "use asm";
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
  var a = 0;
  b();
  return;
 }
})(Module.asmGlobalArg, Module.asmLibraryArg, buffer);



