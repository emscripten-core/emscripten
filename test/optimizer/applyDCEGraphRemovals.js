var name;
var wasmImports = { 'save1': 1, 'number': 33, 'name': name, 'func': function() {}, 'save2': 2 };

// exports gotten directly
var expD1 = Module['expD1'] = asm['expD1'];
var expD2 = Module['expD2'] = asm['expD2'];
var expD3 = Module['expD3'] = asm['expD3'];
var expD4 = Module['expD4'] = asm['expD4'];
// Like above, but not exported on the Module
var expD5 = asm['expD5'];
var expD6 = asm['expD6'];

// exports gotten indirectly (async compilation
var expI1 = Module['expI1'] = (function() {
 return (expI1 = Module['expI1'] = Module['asm']['expI1']).apply(null, arguments);
});
var expI2 = Module['expI2'] = (function() {
 return (expI2 = Module['expI2'] = Module['asm']['expI2']).apply(null, arguments);
});
var expI3 = Module['expI3'] = (function() {
 return (expI3 = Module['expI3'] = Module['asm']['expI3']).apply(null, arguments);
});
var expI4 = Module['expI4'] = (function() {
 return (expI4 = Module['expI4'] = Module['asm']['expI4']).apply(null, arguments);
});
// Like above, but not exported on the Module
var expI5 = (function() {
 return (expI5 = Module['asm']['expI5']).apply(null, arguments);
});
var expI6 = (function() {
 return (expI6 = Module['asm']['expI6']).apply(null, arguments);
});

// add uses for some of them, leave *4 as non-roots
expD1;
Module['expD2'];
asm['expD3'];

expI1;
Module['expI2'];
asm['expI3'];

// EXTRA_INFO: { "unused": ["emcc$import$number", "emcc$import$name", "emcc$import$func", "emcc$export$expD4", "emcc$export$expD6", "emcc$export$expI4", "emcc$export$expI6"] }
