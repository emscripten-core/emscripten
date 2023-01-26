var name;

var wasmImports = {
 "save1": 1,
 "save2": 2
};

var expD1 = Module["expD1"] = asm["expD1"];

var expD2 = Module["expD2"] = asm["expD2"];

var expD3 = Module["expD3"] = asm["expD3"];

var expD4;

var expD5 = asm["expD5"];

var expD6;

var expI1 = Module["expI1"] = function() {
 return (expI1 = Module["expI1"] = Module["asm"]["expI1"]).apply(null, arguments);
};

var expI2 = Module["expI2"] = function() {
 return (expI2 = Module["expI2"] = Module["asm"]["expI2"]).apply(null, arguments);
};

var expI3 = Module["expI3"] = function() {
 return (expI3 = Module["expI3"] = Module["asm"]["expI3"]).apply(null, arguments);
};

var expI4;

var expI5 = function() {
 return (expI5 = Module["asm"]["expI5"]).apply(null, arguments);
};

var expI6;

expD1;

Module["expD2"];

asm["expD3"];

expI1;

Module["expI2"];

asm["expI3"];
