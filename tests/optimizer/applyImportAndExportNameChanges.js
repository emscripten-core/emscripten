var name;
Module.asmLibraryArg = { 'save1': 1, 'number': 33 };

// exports
var expD1 = Module['expD1'] = asm['expD1'];

// exports gotten indirectly (async compilation
var expI1 = Module['expI1'] = (function() {
 return Module['asm']['expI1'].apply(null, arguments);
});

// EXTRA_INFO: { "mapping": {"save1" : "a", "number": "b", "expD1": "c", "expI1": "d" }}
