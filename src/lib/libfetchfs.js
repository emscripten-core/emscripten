/**
 * @license
 * Copyright 2023 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $FETCHFS__deps: ['$stringToUTF8OnStack', 'wasmfs_create_fetch_backend'],
  $FETCHFS: {
    createBackend(opts) {
      return withStackSave(
        () => _wasmfs_create_fetch_backend(
          stringToUTF8OnStack(opts.base_url ?? ""),
          opts.chunkSize | 0
        )
      );
    },
  },
});

if (!WASMFS) {
  error("using -lfetchfs.js requires using WasmFS (-sWASMFS)");
}
