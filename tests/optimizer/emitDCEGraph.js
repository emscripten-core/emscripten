var temp;
Module.asmLibraryArg = { 'temp': temp };

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

// add uses for some of them, leave *4 as non-roots
expD1;
Module['expD2'];

expI1;
Module['expI2'];

