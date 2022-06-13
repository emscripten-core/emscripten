var name;
var asmLibraryArg = { 'save1': 1, 'number': 33, 'name': name, 'func': function() {}, 'save2': 2 };

// exports gotten directly in the minimal runtime style
WebAssembly.instantiate(Module["wasm"], imports).then(function(output) {
 asm = output.instance.exports;
 expD1 = asm['expD1'];
 expD2 = asm['expD2'];
 expD3 = asm['expD3'];
 expD4 = asm['expD4'];
 initRuntime(asm);
 ready();
});

// add uses for some of them, leave *4 as non-roots
expD1;
Module['expD2'];
asm['expD3'];

// EXTRA_INFO: { "unused": ["emcc$import$number", "emcc$import$name", "emcc$import$func", "emcc$export$expD4", "emcc$export$expI4"] }
