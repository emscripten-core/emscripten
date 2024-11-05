/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// This implementation ensures that Windows-style paths are being
// used when running on a Windows operating system - see:
// https://nodejs.org/api/path.html#path_windows_vs_posix
// It's only used/needed when linking with `-sNODERAWFS`, as that
// will replace all normal filesystem access with direct Node.js
// operations. Hence, using `nodePath` should be safe here.

addToLibrary({
  $PATH: {
    isAbs: (path) => nodePath['isAbsolute'](path),
    normalize: (path) => nodePath['normalize'](path),
    dirname: (path) => nodePath['dirname'](path),
    basename: (path) => nodePath['basename'](path),
    join: (...args) => nodePath['join'](...args),
    join2: (l, r) => nodePath['join'](l, r),
  },
  // The FS-using parts are split out into a separate object, so simple path
  // usage does not require the FS.
  $PATH_FS__deps: ['$FS'],
  $PATH_FS__docs: '/** @type{{resolve: function(...*)}} */',
  $PATH_FS: {
    resolve: (...paths) => {
      paths.unshift(FS.cwd());
      return nodePath['posix']['resolve'](...paths);
    },
    relative: (from, to) => nodePath['posix']['relative'](from || FS.cwd(), to || FS.cwd()),
  }
});
