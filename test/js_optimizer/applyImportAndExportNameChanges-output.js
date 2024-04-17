var name;

var wasmImports = {
  a: 1,
  A: 33,
  b: __syscall6,
  __setErrNo: __setErrNo,
  memset: memset,
  sbrk: sbrk,
  memcpy: memcpy,
  _emscripten_memcpy_js: _emscripten_memcpy_js,
  c: __syscall54,
  d: __syscall140,
  q: __syscall146
};

var expD1 = Module["expD1"] = wasmExports["c"];

var expI1 = Module["expI1"] = function() {
  return wasmExports["d"].apply(null, arguments);
};
