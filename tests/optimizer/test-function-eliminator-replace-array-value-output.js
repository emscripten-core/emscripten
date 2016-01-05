// EMSCRIPTEN_START_ASM
var asm = (function(global, env, buffer) {
 "use asm";
 
// EMSCRIPTEN_START_FUNCS
function d() {
 a();
 e();
 return;
}

function c() {
 a();
 return;
}

function a() {
 return 0;
}

// EMSCRIPTEN_END_FUNCS

 var f = [ a ];
})(Module.asmGlobalArg, Module.asmLibraryArg, buffer);
// EMSCRIPTEN_END_ASM
