var name;
Module.asmLibraryArg = {
 "a": 1,
 "b": 33
};
var expD1 = Module["expD1"] = asm["c"];
var expI1 = Module["expI1"] = (function() {
 return Module["asm"]["d"].apply(null, arguments);
});



