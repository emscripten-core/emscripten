var tempNum;

function tempFunc() {
}

function rootedFunc1() {
}
Module['rootedFunc1'] = rootedFunc1;

function rootedFunc2() {
}
var use = rootedFunc2();

function user() {
  used();
}
function used() {
}

function useExportA() {
  expD7();
}

function useExportB() {
  Module['expD7']();
}

function useManySorted() {
  Module['expD9']();
  expD8();
  expD7();
  Module['expD7']();
}

var wasmImports = {
  'tempNum': tempNum,
  'tempFunc': tempFunc,
  'rootedFunc1': rootedFunc1,
  'rootedFunc2': rootedFunc2,
  'user': user,
  'used': used,
  'useExportA': useExportA,
  'useExportB': useExportB,
  'useManySorted': useManySorted,
};

// exports gotten directly
var expD1 = Module['expD1'] = wasmExports['expD1'];
var expD2 = Module['expD2'] = wasmExports['expD2'];
var expD3 = Module['expD3'] = wasmExports['expD3'];
var expD4 = Module['expD4'] = wasmExports['expD4'];
// Same as above but not export on the Module
var expD5 = wasmExports['expD5'];
var expD6 = wasmExports['expD6'];
var expD7 = wasmExports['expD7'];
var expD8 = wasmExports['expD8'];
var expD9 = wasmExports['expD9'];

function applySignatureConversions() {
  // Wrapping functions should not constitute a usage
  wasmExports['expD6'] = foo(wasmExports['expD6']);
}

// add uses for some of them
expD1;
Module['expD2'];
wasmExports['expD3'];

// deep uses, that we can't scan
function usedFromDeep() {
}
var Something = {
  property: function() {
    usedFromDeep();
  },
};

function usedFromDeep2() {
}
var func = function() {
  usedFromDeep2();
};

// dyncalls
var dynCall_v = Module["dynCall_v"] = () => wasmExports["dynCall_v"]();
var dynCall_vi = Module["dynCall_vi"] = () => wasmExports["dynCall_vi"]();
var dynCall_vii = Module["dynCall_vii"] = () => wasmExports["dynCall_vii"]();
var dynCall_viii = Module["dynCall_viii"] = () => wasmExports["dynCall_viii"]();

dynCall_v(ptr); // use directly
Module['dynCall_vi'](ptr, 1); // use on module
dynCall('vii', ptr, [2, 3]); // use indirectly, depending on analysis of dynCall(string, )
// and viii is never used, so definitely legitimately eliminatable

// Don't crash on this code pattern, which regressed in #10724
(function(output) {
  x++;
});

// Don't crash on this code pattern, which we should ignore.
var _bad = function() {
  return something();
};
