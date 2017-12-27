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

Module.asmLibraryArg = {
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

expI1;
Module['expI2'];

// add some defuns TODO
