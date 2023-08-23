/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $OPFS__deps: ['wasmfs_create_opfs_backend'],
  $OPFS: {
    createBackend(opts) {
      return _wasmfs_create_opfs_backend();
    }
  },
});

if (!WASMFS) {
  error("using -lopfs.js requires using WasmFS (-sWASMFS)");
}
