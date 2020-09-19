#if RELOCATABLE // normal static binaries export the table
var wasmTable = new WebAssembly.Table({
  'initial': {{{ WASM_TABLE_SIZE }}},
#if !ALLOW_TABLE_GROWTH
  'maximum': {{{ WASM_TABLE_SIZE }}},
#endif
  'element': 'anyfunc'
});
#endif
