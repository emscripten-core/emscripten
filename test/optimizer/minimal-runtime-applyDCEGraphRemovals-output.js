var name;

var wasmImports = {
 save1: 1,
 save2: 2
};

WebAssembly.instantiate(Module["wasm"], imports).then(output => {
 wasmExports = output.instance.exports;
 expD1 = wasmExports["expD1"];
 expD2 = wasmExports["expD2"];
 expD3 = wasmExports["expD3"];
 initRuntime(wasmExports);
 ready();
});

expD1;

Module["expD2"];

wasmExports["expD3"];
