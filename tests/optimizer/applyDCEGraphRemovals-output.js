var name;

var asmLibraryArg = {
 "save1": 1,
 "save2": 2
};

var expD1 = Module["expD1"] = asm["expD1"];

var expD2 = Module["expD2"] = asm["expD2"];

var expD3 = Module["expD3"] = asm["expD3"];

var expD4;

var expI1 = Module["expI1"] = function() {
 return Module["asm"]["expI1"].apply(null, arguments);
};

var expI2 = Module["expI2"] = function() {
 return Module["asm"]["expI2"].apply(null, arguments);
};

var expI3 = Module["expI3"] = function() {
 return Module["asm"]["expI3"].apply(null, arguments);
};

var expI4;

expD1;

Module["expD2"];

asm["expD3"];

expI1;

Module["expI2"];

asm["expI3"];
