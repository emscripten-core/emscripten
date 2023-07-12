/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $FETCHFS__deps: ['$stringToUTF8OnStack', 'wasmfs_create_fetch_backend'],
  $FETCHFS: {
    createBackend(opts) {
      return _wasmfs_create_fetch_backend(stringToUTF8OnStack(opts.base_url));
    }
  },
});
