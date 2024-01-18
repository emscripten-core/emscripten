var name;
var wasmImports = { save1: 1, number: 33, name: name, func: function() {}, save2: 2 };

// exports gotten directly in the minimal runtime style
WebAssembly.instantiate(Module["wasm"], imports).then((output) => {
 wasmExports = output.instance.exports;
 expD1 = wasmExports['expD1'];
 expD2 = wasmExports['expD2'];
 expD3 = wasmExports['expD3'];
 expD4 = wasmExports['expD4'];
 initRuntime(wasmExports);
 ready();
});

// add uses for some of them, leave *4 as non-roots
expD1;
Module['expD2'];
wasmExports['expD3'];

// EXTRA_INFO: { "unused": ["emcc$import$number", "emcc$import$name", "emcc$import$func", "emcc$export$expD4", "emcc$export$expI4"] }
