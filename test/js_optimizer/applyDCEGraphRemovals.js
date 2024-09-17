var name;
var wasmImports = {
  save1: 1,
  number: 33,
  name: name,
  func: function() {},
  save2: 2
};

// exports gotten directly
var _expD1 = Module['_expD1'] = wasmExports['expD1'];
var _expD2 = Module['_expD2'] = wasmExports['expD2'];
var _expD3 = Module['_expD3'] = wasmExports['expD3'];
var _expD4 = Module['_expD4'] = wasmExports['expD4'];
// Like above, but not exported on the Module
var _expD5 = wasmExports['expD5'];
var _expD6 = wasmExports['expD6'];

// exports gotten indirectly (async compilation
var _expI1 = Module['_expI1'] = () => (expI1 = Module['_expI1'] = wasmExports['expI1'])();
var _expI2 = Module['_expI2'] = () => (expI2 = Module['_expI2'] = wasmExports['expI2'])();
var _expI3 = Module['_expI3'] = () => (expI3 = Module['_expI3'] = wasmExports['expI3'])();
var _expI4 = Module['_expI4'] = () => (expI4 = Module['_expI4'] = wasmExports['expI4'])();

// Like above, but not exported on the Module
var _expI5 = () => (_expI5 = wasmExports['expI5'])();
var _expI6 = () => (_expI6 = wasmExports['expI6'])();

// add uses for some of them, leave *4 as non-roots
_expD1;
Module['_expD2'];
wasmExports['_expD3'];

_expI1;
Module['_expI2'];
wasmExports['_expI3'];

// EXTRA_INFO: { "unusedImports": ["number", "name", "func"], "unusedExports": ["expD4", "expD6", "expI4", "expI6"] }
