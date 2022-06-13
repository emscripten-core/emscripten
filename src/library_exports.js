/**
 * @license
 * Copyright 2020 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  emscripten_get_exported_function__sig: 'pp',
  emscripten_get_exported_function: function(name) {
    name = UTF8ToString(name);
    // Wasm backend does not use C name mangling on exports,
    // so adjust for that manually.
    if (name[0] == '_') name = name.substr(1);
#if MINIMAL_RUNTIME
    var exportedFunc = asm[name];
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
    err('No exported function found by name "' + exportedFunc + '"');
#endif
    // implicit return 0;
  }
});
