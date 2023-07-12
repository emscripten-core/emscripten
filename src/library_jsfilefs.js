/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $JSFILEFS__deps: ['wasmfs_create_js_file_backend'],
  $JSFILEFS: {
    createBackend(opts) {
      return _wasmfs_create_js_file_backend();
    }
  },
});
