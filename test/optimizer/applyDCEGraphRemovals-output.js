var name;

var wasmImports = {
 save1: 1,
 save2: 2
};

var expD1 = Module["expD1"] = asm["expD1"];

var expD2 = Module["expD2"] = asm["expD2"];

var expD3 = Module["expD3"] = asm["expD3"];

var expD4;

var expD5 = asm["expD5"];

var expD6;

var expI1 = Module["expI1"] = () => (expI1 = Module["expI1"] = wasmExports["expI1"])();

var expI2 = Module["expI2"] = () => (expI2 = Module["expI2"] = wasmExports["expI2"])();

var expI3 = Module["expI3"] = () => (expI3 = Module["expI3"] = wasmExports["expI3"])();

var expI4;

var expI5 = () => (expI5 = wasmExports["expI5"])();

var expI6;

expD1;

Module["expD2"];

asm["expD3"];

expI1;

Module["expI2"];

asm["expI3"];
