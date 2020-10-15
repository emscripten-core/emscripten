#if RELOCATABLE
// In RELOCATABLE mode we create the table in JS.
var wasmTable = new WebAssembly.Table({
  'initial': {{{ WASM_TABLE_SIZE }}},
#if !ALLOW_TABLE_GROWTH
  'maximum': {{{ WASM_TABLE_SIZE }}},
#endif
  'element': 'anyfunc'
});
#else
// In regular non-RELOCATABLE mode the table is exported
// from the wasm module and this will be assigned once
// the exports are available.
var wasmTable;
#endif
