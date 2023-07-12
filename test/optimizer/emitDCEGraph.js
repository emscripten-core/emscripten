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
  expI3();
}

function useExportB() {
  Module['expI3']();
}

function useManySorted() {
  Module['expI4']();
  expI3();
  expI1();
  Module['expI2']();
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
var expD1 = Module['expD1'] = asm['expD1'];
var expD2 = Module['expD2'] = asm['expD2'];
var expD3 = Module['expD3'] = asm['expD3'];
var expD4 = Module['expD4'] = asm['expD4'];
// Same as above but not export on the Module
var expD5 = asm['expD5'];

// exports gotten indirectly (async compilation
var expI1 = Module['expI1'] = () => (expI1 = Module['expI1'] = Module['asm']['expI1'])();
var expI2 = Module['expI2'] = () => (expI2 = Module['expI2'] = Module['asm']['expI2'])();
var expI3 = Module['expI3'] = () => (expI3 = Module['expI3'] = Module['asm']['expI3'])();
var expI4 = Module['expI4'] = () => (expI4 = Module['expI4'] = Module['asm']['expI4'])();

// Same as above but not export on the Module.
var expI5 = () => (expI5 = Module['asm']['expI5'])();

// add uses for some of them
expD1;
Module['expD2'];
asm['expD3'];

expI1;
Module['expI2'];
Module['asm']['expI3'];

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
var dynCall_v = Module["dynCall_v"] = () => Module["asm"]["dynCall_v"]();
var dynCall_vi = Module["dynCall_vi"] = () => Module["asm"]["dynCall_vi"]();
var dynCall_vii = Module["dynCall_vii"] = () => Module["asm"]["dynCall_vii"]();
var dynCall_viii = Module["dynCall_viii"] = () => Module["asm"]["dynCall_viii"]();

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
