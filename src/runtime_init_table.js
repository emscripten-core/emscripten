#if !STANDALONE_WASM // standalone wasm creates the table in the wasm
var wasmTable = new WebAssembly.Table({
  'initial': {{{ getQuoted('WASM_TABLE_SIZE') }}},
#if !ALLOW_TABLE_GROWTH
#if WASM_BACKEND
  'maximum': {{{ getQuoted('WASM_TABLE_SIZE') }}} + {{{ RESERVED_FUNCTION_POINTERS }}},
#else
  'maximum': {{{ getQuoted('WASM_TABLE_SIZE') }}},
#endif
#endif // WASM_BACKEND
  'element': 'anyfunc'
});
#endif
