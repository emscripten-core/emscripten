Module['instantiateWasm'] = async (imports, successCallback) => {
  wasmBinaryFile ??= findWasmBinary();
  var {instance, module} = await instantiateArrayBuffer(wasmBinaryFile, imports);
  successCallback(instance, module);
};
