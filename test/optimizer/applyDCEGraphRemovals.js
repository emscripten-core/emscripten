var name;
var wasmImports = { save1: 1, number: 33, name: name, func: function() {}, save2: 2 };

// exports gotten directly
var expD1 = Module['expD1'] = asm['expD1'];
var expD2 = Module['expD2'] = asm['expD2'];
var expD3 = Module['expD3'] = asm['expD3'];
var expD4 = Module['expD4'] = asm['expD4'];
// Like above, but not exported on the Module
var expD5 = asm['expD5'];
var expD6 = asm['expD6'];

// exports gotten indirectly (async compilation
var expI1 = Module['expI1'] = () => (expI1 = Module['expI1'] = wasmExports['expI1'])();
var expI2 = Module['expI2'] = () => (expI2 = Module['expI2'] = wasmExports['expI2'])();
var expI3 = Module['expI3'] = () => (expI3 = Module['expI3'] = wasmExports['expI3'])();
var expI4 = Module['expI4'] = () => (expI4 = Module['expI4'] = wasmExports['expI4'])();

// Like above, but not exported on the Module
var expI5 = () => (expI5 = wasmExports['expI5'])();
var expI6 = () => (expI6 = wasmExports['expI6'])();

// add uses for some of them, leave *4 as non-roots
expD1;
Module['expD2'];
asm['expD3'];

expI1;
Module['expI2'];
asm['expI3'];

// EXTRA_INFO: { "unused": ["emcc$import$number", "emcc$import$name", "emcc$import$func", "emcc$export$expD4", "emcc$export$expD6", "emcc$export$expI4", "emcc$export$expI6"] }
