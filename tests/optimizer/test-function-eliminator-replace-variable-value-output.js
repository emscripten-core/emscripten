var asm = (function(global, env, buffer) {
 "use asm";
 function a() {
  return 0;
 }
 function c() {
  a();
  return;
 }
 function d() {
  a();
  var e = a;
  e();
  return;
 }
})(asmGlobalArg, asmLibraryArg, buffer);
