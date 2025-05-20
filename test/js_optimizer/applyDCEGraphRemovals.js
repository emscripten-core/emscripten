var name;
var wasmImports = {
  save1: 1,
  number: 33,
  name: name,
  func: function() {},
  save2: 2
};

var _expD1;
var _expD2;
var _expD3;
var _expD4;
var _expD5;
var _expD6;

function assignWasmExports(wasmExports) {
  Module['_expD1'] = _expD1 = wasmExports['expD1'];
  Module['_expD2'] = _expD2 = wasmExports['expD2'];
  Module['_expD3'] = _expD3 = wasmExports['expD3'];
  Module['_expD4'] = _expD4 = wasmExports['expD4'];
  // Like above, but not exported on the Module
  _expD5 = wasmExports['expD5'];
  _expD6 = wasmExports['expD6'];
}

// add uses for some of them, leave *4 as non-roots
_expD1;
Module['_expD2'];
wasmExports['_expD3'];

// EXTRA_INFO: { "unusedImports": ["number", "name", "func"], "unusedExports": ["expD4", "expD6"] }
