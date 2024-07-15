var name;
var wasmImports = {
  save1: 1,
  number: 33,
  __syscall6: ___syscall6,
  __setErrNo: ___setErrNo,
  memset: _memset,
  sbrk: _sbrk,
  memcpy: _memcpy,
  emscripten_memcpy_js: _emscripten_memcpy_js,
  __syscall54: ___syscall54,
  __syscall140: ___syscall140,
  __syscall146: ___syscall146
};

// exports
var expD1 = Module['expD1'] = wasmExports['expD1'];

// exports gotten indirectly (async compilation
var expI1 = Module['expI1'] = (function() {
 return wasmExports['expI1'].apply(null, arguments);
});

// EXTRA_INFO: { "mapping": {"save1" : "a", "number": "A", "expD1": "c", "expI1": "d", "__wasm_call_ctors": "e", "stackRestore": "h", "stackAlloc": "g", "__syscall140": "d", "main": "f", "__syscall146": "q", "__syscall54": "c", "__syscall6": "b" }}

