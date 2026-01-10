/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  emscripten_get_exported_function__deps: ['$addFunction', '$UTF8ToString'],
  emscripten_get_exported_function: (name) => {
    name = UTF8ToString(name);
    // Wasm backend does not use C name mangling on exports,
    // so adjust for that manually.
    if (name[0] == '_') name = name.slice(1);
    var exportedFunc = wasmExports[name];
    if (exportedFunc) {
      // Note: addFunction automatically caches the created function pointer.
      return addFunction(exportedFunc);
    }
#if ASSERTIONS
    err(`No exported function found by name "{exportedFunc}"`);
#endif
    // implicit return 0;
  }
});
