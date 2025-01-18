/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $ICASEFS__deps: ['wasmfs_create_icase_backend'],
  $ICASEFS: {
    createBackend(opts) {
      if (typeof opts.backend === "undefined") {
        throw new Error("Underlying backend is not valid.");
      }
      var underlyingBackend = opts.backend.createBackend(opts);
      return _wasmfs_create_icase_backend(underlyingBackend);
    }
  },
});

if (!WASMFS) {
  error("using -licasefs.js requires using WasmFS (-sWASMFS)");
}
