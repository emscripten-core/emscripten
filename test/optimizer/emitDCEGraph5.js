// wasm backend notation has one fewer _ in the wasm
__GLOBAL__I_000101(); // var use
var __GLOBAL__I_000101 = Module["__GLOBAL__I_000101"] = function() { return Module["asm"]["_GLOBAL__I_000101"].apply(null, arguments) };

__ATINIT__.push({ func: function() { __GLOBAL__I_iostream() } }); // var use inside other scope
var __GLOBAL__I_iostream = Module["__GLOBAL__I_iostream"] = function() { return Module["asm"]["_GLOBAL__I_iostream.cpp"].apply(null, arguments) }; // also "." => "_"

Module["__DUB"](); // module use
var __DUB = Module["__DUB"] = function() { return Module["asm"]["_DUB"].apply(null, arguments) };

var __UNUSED = Module["__UNUSED"] = function() { return Module["asm"]["_UNUSED"].apply(null, arguments) };

var asmLibraryArg = {
};

