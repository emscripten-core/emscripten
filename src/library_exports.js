mergeInto(LibraryManager.library, {
  emscripten_get_exported_function: function(name) {
    name = UTF8ToString(name);
#if WASM_BACKEND
    // Wasm backend does not use C name mangling on exports,
    // so adjust for that manually.
    if (name[0] == '_') name = name.substr(1);
#endif
#if MINIMAL_RUNTIME
#if !WASM && !WASM_BACKEND
    // In old fastcomp asm.js, exports are available in outermost scope
    // under var asm.
    var exportedFunc = asm[name];
#else
    // In wasm builds, exports are captured to a dedicated variable
    // wasmExports.
    var exportedFunc = wasmExports[name];
#endif
#else
    // In regular runtime, exports are available on the Module object.
    var exportedFunc = Module["asm"][name];
#endif
    if (exportedFunc) {
      // Record the created function pointer to each function object,
      // so that if the same function pointer is obtained several times,
      // the same address will be returned.
      if (!exportedFunc.ptr) exportedFunc.ptr = addFunction(exportedFunc);
      return exportedFunc.ptr;
    }
#if ASSERTIONS
    console.error('No exported function found by name "' + exportedFunc + '"');
#endif
    // implicit return 0;
  }
});
