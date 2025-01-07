var name;

var wasmImports = {
  save1: 1,
  save2: 2
};

var _expD1 = Module["_expD1"] = wasmExports["expD1"];

var _expD2 = Module["_expD2"] = wasmExports["expD2"];

var _expD3 = Module["_expD3"] = wasmExports["expD3"];

var _expD5 = wasmExports["expD5"];

var _expI1 = Module["_expI1"] = () => (expI1 = Module["_expI1"] = wasmExports["expI1"])();

var _expI2 = Module["_expI2"] = () => (expI2 = Module["_expI2"] = wasmExports["expI2"])();

var _expI3 = Module["_expI3"] = () => (expI3 = Module["_expI3"] = wasmExports["expI3"])();

var _expI5 = () => (_expI5 = wasmExports["expI5"])();

_expD1;

Module["_expD2"];

wasmExports["_expD3"];

_expI1;

Module["_expI2"];

wasmExports["_expI3"];
