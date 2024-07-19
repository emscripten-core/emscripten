var name;

var wasmImports = {
  save1: 1,
  save2: 2
};

WebAssembly.instantiate(Module["wasm"], imports).then(output => {
  wasmExports = output.instance.exports;
  _expD1 = wasmExports["expD1"];
  _expD2 = wasmExports["expD2"];
  _expD3 = wasmExports["expD3"];
  initRuntime(wasmExports);
  ready();
});

_expD1;

Module["_expD2"];

wasmExports["_expD3"];
