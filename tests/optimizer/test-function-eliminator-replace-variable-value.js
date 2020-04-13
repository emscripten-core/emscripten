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
  b();
  var e = b;
  e();
  return;
 }
})(asmGlobalArg, asmLibraryArg, buffer);



