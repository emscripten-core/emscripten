/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

mergeInto(LibraryManager.library, {
  $wasmFSPreloadedFiles: [],
  $wasmFSPreloadedDirs: [],
  $FS__postset: `
FS.init();
FS.createPreloadedFile = FS_createPreloadedFile;
`,
  $FS__deps: [
    '$wasmFSPreloadedFiles',
    '$wasmFSPreloadedDirs',
    '$PATH',
    '$stringToUTF8OnStack',
    '$withStackSave',
    '$readI53FromI64',
    '$readI53FromU64',
    '$FS_createPreloadedFile',
    '$FS_getMode',
    // For FS.readFile
    '$UTF8ArrayToString',
#if FORCE_FILESYSTEM
    '$FS_modeStringToFlags',
    'malloc',
    'free',
#endif
  ],
  $FS : {
    init: () => {
      FS.ensureErrnoError();
    },
    ErrnoError: null,
    handleError: (returnValue) => {
      // Assume errors correspond to negative returnValues
      // since some functions like _wasmfs_open() return positive
      // numbers on success (some callers of this function may need to negate the parameter).
      if (returnValue < 0) {
        throw new FS.ErrnoError(-returnValue);
      }

      return returnValue;
    },
    ensureErrnoError: () => {
      if (FS.ErrnoError) return;
      FS.ErrnoError = /** @this{Object} */ function ErrnoError(code) {
        this.errno = code;
        this.message = 'FS error';
        this.name = "ErrnoError";
      }
      FS.ErrnoError.prototype = new Error();
      FS.ErrnoError.prototype.constructor = FS.ErrnoError;
    },
    createDataFile: (parent, name, fileData, canRead, canWrite, canOwn) => {
      // Data files must be cached until the file system itself has been initialized.
      var mode = FS_getMode(canRead, canWrite);
      var pathName = name ? parent + '/' + name : parent;
      wasmFSPreloadedFiles.push({pathName, fileData, mode});
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

      return ret;
    },

    // FS.cwd is in the awkward position of being used from various JS
    // libraries through PATH_FS. FORCE_FILESYSTEM may not have been set while
    // using those libraries, which means we cannot put this method in the
    // ifdef for that setting just below. Instead, what we can use to tell if we
    // need this method is whether the compiled get_cwd() method is present, as
    // we include that both when FORCE_FILESYSTEM *and* when PATH_FS is in use
    // (see the special PATH_FS deps logic for WasmFS).
    //
    // While this may seem odd, it also makes sense: we include this JS method
    // exactly when the wasm method it wants to call is present.
#if hasExportedSymbol('_wasmfs_get_cwd')
    cwd: () => UTF8ToString(__wasmfs_get_cwd()),
#endif

#if FORCE_FILESYSTEM
    // Full JS API support

    mkdir: (path, mode) => withStackSave(() => {
      mode = mode !== undefined ? mode : 511 /* 0777 */;
      var buffer = stringToUTF8OnStack(path);
      return __wasmfs_mkdir({{{ to64('buffer') }}}, mode);
    }),
    // TODO: mkdirTree
    rmdir: (path) => FS.handleError(
      withStackSave(() => __wasmfs_rmdir(stringToUTF8OnStack(path)))
    ),
    open: (path, flags, mode) => withStackSave(() => {
      flags = typeof flags == 'string' ? FS_modeStringToFlags(flags) : flags;
      mode = typeof mode == 'undefined' ? 438 /* 0666 */ : mode;
      var buffer = stringToUTF8OnStack(path);
      var fd = FS.handleError(__wasmfs_open({{{ to64('buffer') }}}, flags, mode));
      return { fd : fd };
    }),
    create: (path, mode) => {
      // Default settings copied from the legacy JS FS API.
      mode = mode !== undefined ? mode : 438 /* 0666 */;
      mode &= {{{ cDefs.S_IALLUGO }}};
      mode |= {{{ cDefs.S_IFREG }}};
      return FS.mknod(path, mode, 0);
    },
    close: (stream) => FS.handleError(-__wasmfs_close(stream.fd)),
    unlink: (path) => withStackSave(() => {
      var buffer = stringToUTF8OnStack(path);
      return __wasmfs_unlink(buffer);
    }),
    chdir: (path) => withStackSave(() => {
      var buffer = stringToUTF8OnStack(path);
      return __wasmfs_chdir(buffer);
    }),
    // TODO: read
    read: (stream, buffer, offset, length, position) => {
      var seeking = typeof position != 'undefined';

      var dataBuffer = _malloc(length);

      var bytesRead;
      if (seeking) {
        bytesRead = __wasmfs_pread(stream.fd, dataBuffer, length, position);
      } else {
        bytesRead = __wasmfs_read(stream.fd, dataBuffer, length);
      }
      bytesRead = FS.handleError(bytesRead);

      for (var i = 0; i < length; i++) {
        buffer[offset + i] = {{{ makeGetValue('dataBuffer', 'i', 'i8')}}}
      }

      _free(dataBuffer);
      return bytesRead;
    },
    // Note that canOwn is an optimization that we ignore for now in WasmFS.
    write: (stream, buffer, offset, length, position, canOwn) => {
      var seeking = typeof position != 'undefined';

      var dataBuffer = _malloc(length);
      for (var i = 0; i < length; i++) {
        {{{ makeSetValue('dataBuffer', 'i', 'buffer[offset + i]', 'i8') }}};
      }

      var bytesRead;
      if (seeking) {
        bytesRead = __wasmfs_pwrite(stream.fd, dataBuffer, length, position);
      } else {
        bytesRead = __wasmfs_write(stream.fd, dataBuffer, length);
      }
      bytesRead = FS.handleError(bytesRead);
      _free(dataBuffer);

      return bytesRead;
    },
    // TODO: allocate
    // TODO: mmap
    // TODO: msync
    // TODO: munmap
    writeFile: (path, data) => withStackSave(() => {
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
    }),
    symlink: (target, linkpath) => withStackSave(() => {
      var targetBuffer = stringToUTF8OnStack(target);
      var linkpathBuffer = stringToUTF8OnStack(linkpath);
      return __wasmfs_symlink(targetBuffer, linkpathBuffer);
    }),
    // TODO: readlink
    statBufToObject : (statBuf) => {
      // i53/u53 are enough for times and ino in practice.
      return {
          dev: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_dev, "u32") }}},
          mode: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_mode, "u32") }}},
          nlink: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_nlink, "u32") }}},
          uid: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_uid, "u32") }}},
          gid: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_gid, "u32") }}},
          rdev: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_rdev, "u32") }}},
          size: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_size, "i53") }}},
          blksize: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_blksize, "u32") }}},
          blocks: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_blocks, "u32") }}},
          atime: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_atim.tv_sec, "i53") }}},
          mtime: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_mtim.tv_sec, "i53") }}},
          ctime: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_ctim.tv_sec, "i53") }}},
          ino: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_ino, "u53") }}}
      }
    },
    // TODO: stat
    stat: (path) => {
      var statBuf = _malloc({{{ C_STRUCTS.stat.__size__ }}});
      FS.handleError(withStackSave(() => {
        return __wasmfs_stat(stringToUTF8OnStack(path), statBuf);
      }));
      var stats = FS.statBufToObject(statBuf);
      _free(statBuf);

      return stats;
    },
    // TODO: lstat
    lstat: (path) => {
      var statBuf = _malloc({{{ C_STRUCTS.stat.__size__ }}});
      FS.handleError(withStackSave(() => {
        return __wasmfs_lstat(stringToUTF8OnStack(path), statBuf);
      }));
      var stats = FS.statBufToObject(statBuf);
      _free(statBuf);

      return stats;
    },
    chmod: (path, mode) => {
      return FS.handleError(withStackSave(() => {
        var buffer = stringToUTF8OnStack(path);
        return __wasmfs_chmod(buffer, mode);
      }));
    },
    lchmod: (path, mode) => {
      return FS.handleError(withStackSave(() => {
        var buffer = stringToUTF8OnStack(path);
        return __wasmfs_lchmod(buffer, mode);
      }));
    },
    fchmod: (fd, mode) => {
      return FS.handleError(__wasmfs_fchmod(fd, mode));
    },
    // TDOO: chown
    // TODO: lchown
    // TODO: fchown
    truncate: (path, len) => {
      return FS.handleError(withStackSave(() => (__wasmfs_truncate(stringToUTF8OnStack(path), {{{ splitI64('len') }}}))));
    },
    ftruncate: (fd, len) => {
      return FS.handleError(__wasmfs_ftruncate(fd, {{{ splitI64('len') }}}));
    },
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
    readdir: (path) => withStackSave(() => {
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
    }),
    // TODO: mount
    // TODO: unmount
    // TODO: lookup
    mknod: (path, mode, dev) => {
      return FS.handleError(withStackSave(() => {
        var pathBuffer = stringToUTF8OnStack(path);
        return __wasmfs_mknod(pathBuffer, mode, dev);
      }));
    },
    // TODO: mkdev
    rename: (oldPath, newPath) => {
      return FS.handleError(withStackSave(() => {
        var oldPathBuffer = stringToUTF8OnStack(oldPath);
        var newPathBuffer = stringToUTF8OnStack(newPath);
        return __wasmfs_rename(oldPathBuffer, newPathBuffer);
      }));
    },
    // TODO: syncfs
    llseek: (stream, offset, whence) => {
      return FS.handleError(__wasmfs_llseek(stream.fd, {{{ splitI64('offset') }}}, whence));
    }
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
  },
});
