/*
 * Copyright 2019 The Emscripten Authors.  All rights reserved.
 * Emscripten is available under two separate licenses, the MIT license and the
 * University of Illinois/NCSA Open Source License.  Both these licenses can be
 * found in the LICENSE file.
 */

mergeInto(LibraryManager.library, {
  proc_exit__deps: ['exit'],
  proc_exit: function(code) {
    return _exit(code);
  },
});

