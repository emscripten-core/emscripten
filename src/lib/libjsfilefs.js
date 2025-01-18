/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $JSFILEFS__deps: ['wasmfs_create_js_file_backend'],
  $JSFILEFS: {
    createBackend(opts) {
      return _wasmfs_create_js_file_backend();
    }
  },
});

if (!WASMFS) {
  error("using -ljsfile.js requires using WasmFS (-sWASMFS)");
}
