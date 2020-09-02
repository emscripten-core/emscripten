#if !STANDALONE_WASM // standalone wasm creates the table in the wasm
var wasmTable = new WebAssembly.Table({
  'initial': {{{ getQuoted('WASM_TABLE_SIZE') }}},
#if !ALLOW_TABLE_GROWTH
  'maximum': {{{ getQuoted('WASM_TABLE_SIZE') }}},
#endif
  'element': 'anyfunc'
});
#endif
