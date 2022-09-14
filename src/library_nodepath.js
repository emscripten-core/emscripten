/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $PATH: {
    isAbs: (path) => nodePath['isAbsolute'](path),
    normalize: (path) => nodePath['normalize'](path),
    dirname: (path) => nodePath['dirname'](path),
    basename: (path) => nodePath['basename'](path),
    join: function () {
      return nodePath['join'].apply(null, arguments);
    },
    join2: (l, r) => nodePath['join'](l, r),
  },
  // The FS-using parts are split out into a separate object, so simple path
  // usage does not require the FS.
  $PATH_FS__deps: ['$FS'],
  $PATH_FS__docs: '/** @type{{resolve: function(...*)}} */',
  $PATH_FS: {
    resolve: function () {
      var paths = Array.prototype.slice.call(arguments, 0);
      paths.unshift(FS.cwd());
      return nodePath['posix']['resolve'].apply(null, paths);
    },
    relative: (from, to) => nodePath['posix']['relative'](from || FS.cwd(), to || FS.cwd()),
  }
});
