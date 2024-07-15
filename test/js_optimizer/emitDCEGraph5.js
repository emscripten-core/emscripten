// wasm backend notation has one fewer _ in the wasm
__GLOBAL__I_000101(); // var use
var __GLOBAL__I_000101 = Module["__GLOBAL__I_000101"] = () => wasmExports["_GLOBAL__I_000101"]();

__ATINIT__.push({ func: function() { __GLOBAL__I_iostream() } }); // var use inside other scope
var __GLOBAL__I_iostream = Module["__GLOBAL__I_iostream"] = () => wasmExports["_GLOBAL__I_iostream.cpp"]();

Module["__DUB"](); // module use
var __DUB = Module["__DUB"] = () => wasmExports["_DUB"]();

var __UNUSED = Module["__UNUSED"] = () => wasmExports["_UNUSED"]();

var wasmImports = {
};

