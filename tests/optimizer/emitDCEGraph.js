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

var asmLibraryArg = {
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

// exports gotten indirectly (async compilation
var expI1 = Module['expI1'] = (function() {
 return Module['asm']['expI1'].apply(null, arguments);
});
var expI2 = Module['expI2'] = (function() {
 return Module['asm']['expI2'].apply(null, arguments);
});
var expI3 = Module['expI3'] = (function() {
 return Module['asm']['expI3'].apply(null, arguments);
});
var expI4 = Module['expI4'] = (function() {
 return Module['asm']['expI4'].apply(null, arguments);
});

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
var dynCall_v = Module["dynCall_v"] = function() {  return Module["asm"]["dynCall_v"].apply(null, arguments) };
var dynCall_vi = Module["dynCall_vi"] = function() {  return Module["asm"]["dynCall_vi"].apply(null, arguments) };
var dynCall_vii = Module["dynCall_vii"] = function() {  return Module["asm"]["dynCall_vii"].apply(null, arguments) };
var dynCall_viii = Module["dynCall_viii"] = function() {  return Module["asm"]["dynCall_viii"].apply(null, arguments) };

dynCall_v(ptr); // use directly
Module['dynCall_vi'](ptr, 1); // use on module
dynCall('vii', ptr, [2, 3]); // use indirectly, depending on analysis of dynCall(string, )
// and viii is never used, so definitely legitimately eliminatable

