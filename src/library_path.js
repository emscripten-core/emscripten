/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $PATH: {
    isAbs: (path) => path.charAt(0) === '/',
    // split a filename into [root, dir, basename, ext], unix version
    // 'root' is just a slash, or nothing.
    splitPath: (filename) => {
      var splitPathRe = /^(\/?|)([\s\S]*?)((?:\.{1,2}|[^\/]+?|)(\.[^.\/]*|))(?:[\/]*)$/;
      return splitPathRe.exec(filename).slice(1);
    },
    normalizeArray: (parts, allowAboveRoot) => {
      // if the path tries to go above the root, `up` ends up > 0
      var up = 0;
      for (var i = parts.length - 1; i >= 0; i--) {
        var last = parts[i];
        if (last === '.') {
          parts.splice(i, 1);
        } else if (last === '..') {
          parts.splice(i, 1);
          up++;
        } else if (up) {
          parts.splice(i, 1);
          up--;
        }
      }
      // if the path is allowed to go above the root, restore leading ..s
      if (allowAboveRoot) {
        for (; up; up--) {
          parts.unshift('..');
        }
      }
      return parts;
    },
    normalize: (path) => {
      var isAbsolute = PATH.isAbs(path),
          trailingSlash = path.substr(-1) === '/';
      // Normalize the path
      path = PATH.normalizeArray(path.split('/').filter((p) => !!p), !isAbsolute).join('/');
      if (!path && !isAbsolute) {
        path = '.';
      }
      if (path && trailingSlash) {
        path += '/';
      }
      return (isAbsolute ? '/' : '') + path;
    },
    dirname: (path) => {
      var result = PATH.splitPath(path),
          root = result[0],
          dir = result[1];
      if (!root && !dir) {
        // No dirname whatsoever
        return '.';
      }
      if (dir) {
        // It has a dirname, strip trailing slash
        dir = dir.substr(0, dir.length - 1);
      }
      return root + dir;
    },
    basename: (path) => {
      // EMSCRIPTEN return '/'' for '/', not an empty string
      if (path === '/') return '/';
      path = PATH.normalize(path);
      path = path.replace(/\/$/, "");
      var lastSlash = path.lastIndexOf('/');
      if (lastSlash === -1) return path;
      return path.substr(lastSlash+1);
    },
    join: (...paths) => PATH.normalize(paths.join('/')),
    join2: (l, r) => PATH.normalize(l + '/' + r),
  },
  // The FS-using parts are split out into a separate object, so simple path
  // usage does not require the FS.
  $PATH_FS__deps: [
    '$PATH',
    '$FS',
#if WASMFS
    // In WasmFS, FS.cwd() is implemented via a call into wasm, so we need to
    // add a dependency on that.
    '_wasmfs_get_cwd',
#endif
  ],
  $PATH_FS: {
    resolve: (...args) => {
      var resolvedPath = '',
        resolvedAbsolute = false;
      for (var i = args.length - 1; i >= -1 && !resolvedAbsolute; i--) {
        var path = (i >= 0) ? args[i] : FS.cwd();
        // Skip empty and invalid entries
        if (typeof path != 'string') {
          throw new TypeError('Arguments to path.resolve must be strings');
        } else if (!path) {
          return ''; // an invalid portion invalidates the whole thing
        }
        resolvedPath = path + '/' + resolvedPath;
        resolvedAbsolute = PATH.isAbs(path);
      }
      // At this point the path should be resolved to a full absolute path, but
      // handle relative paths to be safe (might happen when process.cwd() fails)
      resolvedPath = PATH.normalizeArray(resolvedPath.split('/').filter((p) => !!p), !resolvedAbsolute).join('/');
      return ((resolvedAbsolute ? '/' : '') + resolvedPath) || '.';
    },
    relative: (from, to) => {
      from = PATH_FS.resolve(from).substr(1);
      to = PATH_FS.resolve(to).substr(1);
      function trim(arr) {
        var start = 0;
        for (; start < arr.length; start++) {
          if (arr[start] !== '') break;
        }
        var end = arr.length - 1;
        for (; end >= 0; end--) {
          if (arr[end] !== '') break;
        }
        if (start > end) return [];
        return arr.slice(start, end - start + 1);
      }
      var fromParts = trim(from.split('/'));
      var toParts = trim(to.split('/'));
      var length = Math.min(fromParts.length, toParts.length);
      var samePartsLength = length;
      for (var i = 0; i < length; i++) {
        if (fromParts[i] !== toParts[i]) {
          samePartsLength = i;
          break;
        }
      }
      var outputParts = [];
      for (var i = samePartsLength; i < fromParts.length; i++) {
        outputParts.push('..');
      }
      outputParts = outputParts.concat(toParts.slice(samePartsLength));
      return outputParts.join('/');
    }
  }
});
