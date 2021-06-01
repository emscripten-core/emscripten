#if RELOCATABLE
// In RELOCATABLE mode we create the table in JS.
var wasmTable = new WebAssembly.Table({
  'initial': {{{ INITIAL_TABLE }}},
#if !ALLOW_TABLE_GROWTH
  'maximum': {{{ INITIAL_TABLE }}},
#endif
  'element': 'anyfunc'
});
#else
// In regular non-RELOCATABLE mode the table is exported
// from the wasm module and this will be assigned once
// the exports are available.
var wasmTable;
#endif
