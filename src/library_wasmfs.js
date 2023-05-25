/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $wasmFSPreloadedFiles: [],
  $wasmFSPreloadedDirs: [],
  $FS__postset: `
FS.createPreloadedFile = FS_createPreloadedFile;
`,
  $FS__deps: [
    '$wasmFSPreloadedFiles',
    '$wasmFSPreloadedDirs',
    '$PATH',
    '$stringToUTF8OnStack',
    '$withStackSave',
    '$readI53FromI64',
    '$FS_createPreloadedFile',
    '$FS_getMode',
#if FORCE_FILESYSTEM
    '$FS_modeStringToFlags',
    'malloc',
#endif
  ],
  $FS : {
    ErrnoError: null,
    throwError: (errno) => {
      if (!FS.ErrnoError) {
        FS.ErrnoError = /** @this{Object} */ function ErrnoError(code) {
          this.errno = code;
          this.message = 'FS error';
          this.name = "ErrnoError";
        }
        FS.ErrnoError.prototype = new Error();
        FS.ErrnoError.prototype.constructor = FS.ErrnoError;
      }
      throw new FS.ErrnoError(errno);
    },
    handleError: (returnValue) => {
      // Assume errors correspond to negative returnValues
      // since some functions like _wasmfs_open() return positive
      // numbers on success (some callers of this function may need to negate the parameter).
      if (returnValue < 0) {
        FS.throwError(-returnValue);
      }

      return returnValue;
    },
    createDataFile: (parent, name, data, canRead, canWrite, canOwn) => {
      // Data files must be cached until the file system itself has been initialized.
      var mode = FS_getMode(canRead, canWrite);
      var pathName = name ? parent + '/' + name : parent;
      wasmFSPreloadedFiles.push({pathName: pathName, fileData: data, mode: mode});
    },
    createPath: (parent, path, canRead, canWrite) => {
      // Cache file path directory names.
      var parts = path.split('/').reverse();
      while (parts.length) {
        var part = parts.pop();
        if (!part) continue;
        var current = PATH.join2(parent, part);
        wasmFSPreloadedDirs.push({parentPath: parent, childName: part});
        parent = current;
      }
      return current;
    },
    readFile: (path, opts = {}) => {
      opts.encoding = opts.encoding || 'binary';
      if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
        throw new Error('Invalid encoding type "' + opts.encoding + '"');
      }

      // Copy the file into a JS buffer on the heap.
      var buf = withStackSave(() => __wasmfs_read_file(stringToUTF8OnStack(path)));

      // The signed integer length resides in the first 8 bytes of the buffer.
      var length = {{{ makeGetValue('buf', '0', 'i53') }}};

      // Default return type is binary.
      // The buffer contents exist 8 bytes after the returned pointer.
      var ret = new Uint8Array(HEAPU8.subarray(buf + 8, buf + 8 + length));
      if (opts.encoding === 'utf8') {
        ret = UTF8ArrayToString(ret, 0);
      }

      _free(buf);
      return ret;
    },
    cwd: () => {
      // TODO: Remove dependency on FS.cwd().
      // User code should not be using FS.cwd().
      // For file preloading, cwd should be '/' to begin with.
      return '/';
    },

#if FORCE_FILESYSTEM
    // Full JS API support
    mkdir: (path, mode) => {
      return withStackSave(() => {
        mode = mode !== undefined ? mode : 511 /* 0777 */;
        var buffer = stringToUTF8OnStack(path);
        return __wasmfs_mkdir({{{ to64('buffer') }}}, mode);
      });
    },
    // TODO: mkdirTree
    // TDOO: rmdir
    rmdir: (path) => {
      return withStackSave(() => {
        var buffer = stringToUTF8OnStack(path);
        return __wasmfs_rmdir(buffer);
      })
    },
    // TODO: open
    open: (path, flags, mode) => {
      flags = typeof flags == 'string' ? FS_modeStringToFlags(flags) : flags;
      mode = typeof mode == 'undefined' ? 438 /* 0666 */ : mode;
      return withStackSave(() => {
        var buffer = stringToUTF8OnStack(path);
        return FS.handleError(__wasmfs_open({{{ to64('buffer') }}}, flags, mode));
      })
    },
    // TODO: create
    // TODO: close
    close: (fd) => {
      return FS.handleError(-__wasmfs_close(fd));
    },
    unlink: (path) => {
      return withStackSave(() => {
        var buffer = stringToUTF8OnStack(path);
        return __wasmfs_unlink(buffer);
      });
    },
    chdir: (path) => {
      return withStackSave(() => {
        var buffer = stringToUTF8OnStack(path);
        return __wasmfs_chdir(buffer);
      });
    },
    // TODO: read
    // TODO: write
    // TODO: allocate
    // TODO: mmap
    // TODO: msync
    // TODO: munmap
    writeFile: (path, data) => {
      return withStackSave(() => {
        var pathBuffer = stringToUTF8OnStack(path);
        if (typeof data == 'string') {
          var buf = new Uint8Array(lengthBytesUTF8(data) + 1);
          var actualNumBytes = stringToUTF8Array(data, buf, 0, buf.length);
          data = buf.slice(0, actualNumBytes);
        }
        var dataBuffer = _malloc(data.length);
#if ASSERTIONS
        assert(dataBuffer);
#endif
        for (var i = 0; i < data.length; i++) {
          {{{ makeSetValue('dataBuffer', 'i', 'data[i]', 'i8') }}};
        }
        var ret = __wasmfs_write_file(pathBuffer, dataBuffer, data.length);
        _free(dataBuffer);
        return ret;
      });
    },
    symlink: (target, linkpath) => {
      return withStackSave(() => {
        var targetBuffer = stringToUTF8OnStack(target);
        var linkpathBuffer = stringToUTF8OnStack(linkpath);
        return __wasmfs_symlink(targetBuffer, linkpathBuffer);
      });
    },
    // TODO: readlink
    // TODO: stat
    // TODO: lstat
    chmod: (path, mode) => {
      return withStackSave(() => {
        var buffer = stringToUTF8OnStack(path);
        return __wasmfs_chmod(buffer, mode);
      });
    },
    // TODO: lchmod
    // TODO: fchmod
    // TDOO: chown
    // TODO: lchown
    // TODO: fchown
    // TODO: truncate
    // TODO: ftruncate
    // TODO: utime
    findObject: (path) => {
      var result = __wasmfs_identify(path);
      if (result == {{{ cDefs.ENOENT }}}) {
        return null;
      }
      return {
        isFolder: result == {{{ cDefs.EISDIR }}},
        isDevice: false, // TODO: wasmfs support for devices
      };
    },
    readdir: (path) => {
      return withStackSave(() => {
        var pathBuffer = stringToUTF8OnStack(path);
        var entries = [];
        var state = __wasmfs_readdir_start(pathBuffer);
        if (!state) {
          // TODO: The old FS threw an ErrnoError here.
          throw new Error("No such directory");
        }
        var entry;
        while (entry = __wasmfs_readdir_get(state)) {
          entries.push(UTF8ToString(entry));
        }
        __wasmfs_readdir_finish(state);
        return entries;
      });
    }
    // TODO: mount
    // TODO: unmount
    // TODO: lookup
    // TODO: mknod
    // TODO: mkdev
    // TODO: rename
    // TODO: syncfs
    // TODO: llseek
    // TODO: ioctl

#endif
  },
  _wasmfs_get_num_preloaded_files__deps: ['$wasmFSPreloadedFiles'],
  _wasmfs_get_num_preloaded_files: function() {
    return wasmFSPreloadedFiles.length;
  },
  _wasmfs_get_num_preloaded_dirs__deps: ['$wasmFSPreloadedDirs'],
  _wasmfs_get_num_preloaded_dirs: function() {
    return wasmFSPreloadedDirs.length;
  },
  _wasmfs_get_preloaded_file_mode: function(index) {
    return wasmFSPreloadedFiles[index].mode;
  },
  _wasmfs_get_preloaded_parent_path: function(index, parentPathBuffer) {
    var s = wasmFSPreloadedDirs[index].parentPath;
    var len = lengthBytesUTF8(s) + 1;
    stringToUTF8(s, parentPathBuffer, len);
  },
  _wasmfs_get_preloaded_child_path: function(index, childNameBuffer) {
    var s = wasmFSPreloadedDirs[index].childName;
    var len = lengthBytesUTF8(s) + 1;
    stringToUTF8(s, childNameBuffer, len);
  },
  _wasmfs_get_preloaded_path_name__deps: ['$lengthBytesUTF8', '$stringToUTF8'],
  _wasmfs_get_preloaded_path_name: function(index, fileNameBuffer) {
    var s = wasmFSPreloadedFiles[index].pathName;
    var len = lengthBytesUTF8(s) + 1;
    stringToUTF8(s, fileNameBuffer, len);
  },
  _wasmfs_get_preloaded_file_size: function(index) {
    return wasmFSPreloadedFiles[index].fileData.length;
  },
  _wasmfs_copy_preloaded_file_data: function(index, buffer) {
    HEAPU8.set(wasmFSPreloadedFiles[index].fileData, buffer);
  }
});

DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$FS');
