/**
 * @license
 * Copyright 2022 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

addToLibrary({
  $MEMFS__deps: ['wasmfs_create_memory_backend'],
  $MEMFS: {
    createBackend(opts) {
      return _wasmfs_create_memory_backend();
    }
  },
  $wasmFSPreloadedFiles: [],
  $wasmFSPreloadedDirs: [],
  // We must note when preloading has been "flushed", that is, the time at which
  // WasmFS has started up and read the preloaded data. After that time, no more
  // data needs to be preloaded (and it would be invalid to do so, as any
  // further additions to wasmFSPreloadedFiles|Dirs would be ignored).
  $wasmFSPreloadingFlushed: false,
  $wasmFSDevices: {},
  $wasmFSDeviceStreams: {},

  $FS__deps: [
    '$MEMFS',
    '$wasmFSPreloadedFiles',
    '$wasmFSPreloadedDirs',
    '$wasmFSPreloadingFlushed',
    '$PATH',
    '$stringToUTF8OnStack',
    '$withStackSave',
    '$readI53FromI64',
    '$readI53FromU64',
    '$FS_createDataFile',
    '$FS_createPreloadedFile',
    '$FS_preloadFile',
    '$FS_getMode',
    // For FS.readFile
    '$UTF8ArrayToString',
#if FORCE_FILESYSTEM || INCLUDE_FULL_LIBRARY // FULL_LIBRARY will include JS
                                             // code in other places that ends
                                             // up requiring all of our code
                                             // here.
    '$FS_modeStringToFlags',
    '$FS_create',
    '$FS_mknod',
    '$FS_mkdir',
    '$FS_mkdirTree',
    '$FS_writeFile',
    '$FS_unlink',
#if LibraryManager.has('libicasefs.js')
    '$ICASEFS',
#endif
#if LibraryManager.has('libnodefs.js')
    '$NODEFS',
#endif
#if LibraryManager.has('libopfs.js')
    '$OPFS',
#endif
#if LibraryManager.has('libjsfilefs.js')
    '$JSFILEFS',
#endif
#if LibraryManager.has('libfetchfs.js')
    '$FETCHFS',
#endif
    'malloc',
    'free',
    'wasmfs_create_jsimpl_backend',
    '$wasmFS$backends',
    '$wasmFSDevices',
    '$wasmFSDeviceStreams'
#endif
  ],
  $FS : {
    ErrnoError: class extends Error {
      name = 'ErrnoError';
      message = 'FS error';
      constructor(code) {
        super();
        this.errno = code
      }
    },
    handleError(returnValue) {
      // Assume errors correspond to negative returnValues
      // since some functions like _wasmfs_open() return positive
      // numbers on success (some callers of this function may need to negate the parameter).
      if (returnValue < 0) {
        throw new FS.ErrnoError(-returnValue);
      }

      return returnValue;
    },
    createDataFile(parent, name, fileData, canRead, canWrite, canOwn) {
      FS_createDataFile(parent, name, fileData, canRead, canWrite, canOwn);
    },
    createPath(parent, path, canRead, canWrite) {
      // Cache file path directory names.
      var parts = path.split('/').reverse();
      while (parts.length) {
        var part = parts.pop();
        if (!part) continue;
        var current = PATH.join2(parent, part);
        if (!wasmFSPreloadingFlushed) {
          wasmFSPreloadedDirs.push({parentPath: parent, childName: part});
        } else {
          try {
            FS.mkdir(current);
          } catch (e) {
            if (e.errno != {{{ cDefs.EEXIST }}}) throw e;
          }
        }
        parent = current;
      }
      return current;
    },

    createPreloadedFile(parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) {
      return FS_createPreloadedFile(parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish);
    },

    async preloadFile(parent, name, url, canRead, canWrite, dontCreateFile, canOwn, preFinish) {
      return FS_preloadFile(parent, name, url, canRead, canWrite, dontCreateFile, canOwn, preFinish);
    },

#if hasExportedSymbol('_wasmfs_read_file') // Support the JS function exactly
                                           // when the __wasmfs_* function is
                                           // present to be called (otherwise,
                                           // we'd error anyhow). This depends
                                           // on other code including the
                                           // __wasmfs_* method properly.
    readFile(path, opts = {}) {
      opts.encoding = opts.encoding || 'binary';
      if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
        throw new Error(`Invalid encoding type "${opts.encoding}"`);
      }

      var buf, length;
      // Copy the file into a JS buffer on the heap.
      withStackSave(() => {
        var bufPtr = stackAlloc({{{ POINTER_SIZE }}});
        var sizePtr = stackAlloc({{{ POINTER_SIZE }}});
        FS.handleError(-__wasmfs_read_file(stringToUTF8OnStack(path), bufPtr, sizePtr));
        buf = {{{ makeGetValue('bufPtr', '0', '*') }}};
        length = {{{ makeGetValue('sizePtr', '0', 'i53') }}};
      });

      // Default return type is binary.
      // The buffer contents exist 8 bytes after the returned pointer.
      return opts.encoding === 'utf8' ? UTF8ToString(buf, length) : HEAPU8.slice(buf, buf + length);
    },
#endif

#if hasExportedSymbol('_wasmfs_get_cwd') // Similar to readFile, above.
    cwd: () => UTF8ToString(__wasmfs_get_cwd()),
#endif

#if FORCE_FILESYSTEM || INCLUDE_FULL_LIBRARY // see comment above
    // Full JS API support

    analyzePath(path) {
      // TODO: Consider simplifying this API, which for now matches the JS FS.
      var exists = !!FS.findObject(path);
      return {
        exists,
        object: {
          contents: exists ? FS.readFile(path) : null
        }
      };
    },

    // libc methods

    mkdir: (path, mode) => FS_mkdir(path, mode),
    mkdirTree: (path, mode) => FS_mkdirTree(path, mode),
    rmdir: (path) => FS.handleError(
      withStackSave(() => __wasmfs_rmdir(stringToUTF8OnStack(path)))
    ),
    open: (path, flags, mode = 0o666) => withStackSave(() => {
      flags = typeof flags == 'string' ? FS_modeStringToFlags(flags) : flags;
      var buffer = stringToUTF8OnStack(path);
      var fd = FS.handleError(__wasmfs_open(buffer, flags, mode));
      return { fd : fd };
    }),
    create: (path, mode) => FS_create(path, mode),
    close: (stream) => FS.handleError(-__wasmfs_close(stream.fd)),
    unlink: (path) => FS_unlink(path),
    chdir: (path) => withStackSave(() => __wasmfs_chdir(stringToUTF8OnStack(path))),
    read(stream, buffer, offset, length, position) {
      var seeking = typeof position != 'undefined';

      var dataBuffer = _malloc(length);

      var bytesRead;
      if (seeking) {
        bytesRead = __wasmfs_pread(stream.fd, dataBuffer, length, {{{ splitI64('position') }}});
      } else {
        bytesRead = __wasmfs_read(stream.fd, dataBuffer, length);
      }
      if (bytesRead > 0) {
        buffer.set(HEAPU8.subarray(dataBuffer, dataBuffer + bytesRead), offset);
      }

      _free(dataBuffer);
      return FS.handleError(bytesRead);
    },
    // Note that canOwn is an optimization that we ignore for now in WasmFS.
    write(stream, buffer, offset, length, position, canOwn) {
      var seeking = typeof position != 'undefined';

      var dataBuffer = _malloc(length);
      for (var i = 0; i < length; i++) {
        {{{ makeSetValue('dataBuffer', 'i', 'buffer[offset + i]', 'i8') }}};
      }

      var bytesRead;
      if (seeking) {
        bytesRead = __wasmfs_pwrite(stream.fd, dataBuffer, length, {{{ splitI64('position') }}});
      } else {
        bytesRead = __wasmfs_write(stream.fd, dataBuffer, length);
      }
      _free(dataBuffer);
      return FS.handleError(bytesRead);
    },
    writeFile: (path, data) => FS_writeFile(path, data),
    mmap: (stream, length, offset, prot, flags) => {
      var buf = FS.handleError(__wasmfs_mmap(length, prot, flags, stream.fd, {{{ splitI64('offset') }}}));
      return { ptr: buf, allocated: true };
    },
    // offset is passed to msync to maintain backwards compatibility with the legacy JS API but is not used by WasmFS.
    msync: (stream, bufferPtr, offset, length, mmapFlags) => {
#if ASSERTIONS
      assert(offset === 0);
#endif
      // TODO: assert that stream has the fd corresponding to the mapped buffer (bufferPtr).
      return FS.handleError(__wasmfs_msync(bufferPtr, length, mmapFlags));
    },
    munmap: (addr, length) => (
      FS.handleError(__wasmfs_munmap(addr, length))
    ),
    symlink: (target, linkpath) => withStackSave(() => (
      __wasmfs_symlink(stringToUTF8OnStack(target), stringToUTF8OnStack(linkpath))
    )),
    readlink(path) {
      return withStackSave(() => {
        var bufPtr = stackAlloc({{{ POINTER_SIZE }}});
        FS.handleError(__wasmfs_readlink(stringToUTF8OnStack(path), bufPtr));
        var readBuffer = {{{ makeGetValue('bufPtr', '0', '*') }}};
        return UTF8ToString(readBuffer);
      });
    },
    statBufToObject(statBuf) {
      // i53/u53 are enough for times and ino in practice.
      return {
          dev: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_dev, "u32") }}},
          mode: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_mode, "u32") }}},
          nlink: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_nlink, SIZE_TYPE) }}},
          uid: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_uid, "u32") }}},
          gid: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_gid, "u32") }}},
          rdev: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_rdev, "u32") }}},
          size: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_size, "i53") }}},
          blksize: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_blksize, "i32") }}},
          blocks: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_blocks, "i32") }}},
          atime: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_atim.tv_sec, "i53") }}},
          mtime: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_mtim.tv_sec, "i53") }}},
          ctime: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_ctim.tv_sec, "i53") }}},
          ino: {{{ makeGetValue('statBuf', C_STRUCTS.stat.st_ino, "u53") }}}
      }
    },
    stat(path) {
      return withStackSave(() => {
        var statBuf = stackAlloc({{{ C_STRUCTS.stat.__size__ }}});
        FS.handleError(__wasmfs_stat(stringToUTF8OnStack(path), statBuf));
        return FS.statBufToObject(statBuf);
      });
    },
    lstat(path) {
      return withStackSave(() => {
        var statBuf = stackAlloc({{{ C_STRUCTS.stat.__size__ }}});
        FS.handleError(__wasmfs_lstat(stringToUTF8OnStack(path), statBuf));
        return FS.statBufToObject(statBuf);
      });
    },
    chmod(path, mode) {
      return FS.handleError(withStackSave(() => {
        var buffer = stringToUTF8OnStack(path);
        return __wasmfs_chmod(buffer, mode);
      }));
    },
    lchmod(path, mode) {
      return FS.handleError(withStackSave(() => {
        var buffer = stringToUTF8OnStack(path);
        return __wasmfs_lchmod(buffer, mode);
      }));
    },
    fchmod(fd, mode) {
      return FS.handleError(__wasmfs_fchmod(fd, mode));
    },
    utime: (path, atime, mtime) => (
      FS.handleError(withStackSave(() => (
        __wasmfs_utime(stringToUTF8OnStack(path), atime, mtime)
      )))
    ),
    truncate(path, len) {
      return FS.handleError(withStackSave(() => (__wasmfs_truncate(stringToUTF8OnStack(path), {{{ splitI64('len') }}}))));
    },
    ftruncate(fd, len) {
      return FS.handleError(__wasmfs_ftruncate(fd, {{{ splitI64('len') }}}));
    },
    findObject(path) {
      var result = withStackSave(() => __wasmfs_identify(stringToUTF8OnStack(path)));
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
    mount: (type, opts, mountpoint) => {
#if ASSERTIONS
      if (typeof type == 'string') {
        // The filesystem was not included, and instead we have an error
        // message stored in the variable.
        throw type;
      }
#endif
      var backendPointer = type.createBackend(opts);
      return FS.handleError(withStackSave(() => __wasmfs_mount(stringToUTF8OnStack(mountpoint), backendPointer)));
    },
    unmount: (mountpoint) => (
      FS.handleError(withStackSave(() => _wasmfs_unmount(stringToUTF8OnStack(mountpoint))))
    ),
    // TODO: lookup
    mknod: (path, mode, dev) => FS_mknod(path, mode, dev),
    makedev: (ma, mi) => ((ma) << 8 | (mi)),
    registerDevice(dev, ops) {
      var backendPointer = _wasmfs_create_jsimpl_backend();
      var definedOps = {
        userRead: ops.read,
        userWrite: ops.write,

        allocFile: (file) => {
          wasmFSDeviceStreams[file] = {}
        },
        freeFile: (file) => {
          wasmFSDeviceStreams[file] = undefined;
        },
        getSize: (file) => {},
        // Devices cannot be resized.
        setSize: (file, size) => 0,
        read: (file, buffer, length, offset) => {
          var bufferArray = HEAP8.subarray(buffer, buffer + length);
          try {
            var bytesRead = definedOps.userRead(wasmFSDeviceStreams[file], bufferArray, 0, length, offset);
          } catch (e) {
            return -e.errno;
          }
          HEAP8.set(bufferArray, buffer);
          return bytesRead;
        },
        write: (file, buffer, length, offset) => {
          var bufferArray = HEAP8.subarray(buffer, buffer + length);
          try {
            var bytesWritten = definedOps.userWrite(wasmFSDeviceStreams[file], bufferArray, 0, length, offset);
          } catch (e) {
            return -e.errno;
          }
          HEAP8.set(bufferArray, buffer);
          return bytesWritten;
        },
      };

      wasmFS$backends[backendPointer] = definedOps;
      wasmFSDevices[dev] = backendPointer;
    },
    createDevice(parent, name, input, output) {
      if (typeof parent != 'string') {
        // The old API allowed parents to be objects, which do not exist in WasmFS.
        throw new Error("Only string paths are accepted");
      }
      var path = PATH.join2(parent, name);
      var mode = FS_getMode(!!input, !!output);
      FS.createDevice.major ??= 64;
      var dev = FS.makedev(FS.createDevice.major++, 0);
      // Create a fake device with a set of stream ops to emulate
      // the old API's createDevice().
      FS.registerDevice(dev, {
        read(stream, buffer, offset, length, pos /* ignored */) {
          var bytesRead = 0;
          for (var i = 0; i < length; i++) {
            var result;
            try {
              result = input();
            } catch (e) {
              throw new FS.ErrnoError({{{ cDefs.EIO }}});
            }
            if (result === undefined && bytesRead === 0) {
              throw new FS.ErrnoError({{{ cDefs.EAGAIN }}});
            }
            if (result === null || result === undefined) break;
            bytesRead++;
            buffer[offset+i] = result;
          }
          return bytesRead;
        },
        write(stream, buffer, offset, length, pos) {
          for (var i = 0; i < length; i++) {
            try {
              output(buffer[offset+i]);
            } catch (e) {
              throw new FS.ErrnoError({{{ cDefs.EIO }}});
            }
          }
          return i;
        }
      });
      return FS.mkdev(path, mode, dev);
    },
    // mode is an optional argument, which will be set to 0666 if not passed in.
    mkdev(path, mode, dev) {
      if (typeof dev === 'undefined') {
        dev = mode;
        mode = 0o666;
      }

      var deviceBackend = wasmFSDevices[dev];
      if (!deviceBackend) {
        throw new Error("Invalid device ID.");
      }

      return FS.handleError(withStackSave(() => (
        _wasmfs_create_file(stringToUTF8OnStack(path), mode, deviceBackend)
      )));
    },
    rename(oldPath, newPath) {
      return FS.handleError(withStackSave(() => {
        var oldPathBuffer = stringToUTF8OnStack(oldPath);
        var newPathBuffer = stringToUTF8OnStack(newPath);
        return __wasmfs_rename(oldPathBuffer, newPathBuffer);
      }));
    },
    // TODO: syncfs
    llseek(stream, offset, whence) {
      return FS.handleError(__wasmfs_llseek(stream.fd, {{{ splitI64('offset') }}}, whence));
    }
    // TODO: ioctl

#endif
  },

  // Split-out FS.* methods. These are split out for code size reasons, so that
  // we can include the ones we need on demand, rather than put them all on the
  // main FS object. As a result the entire FS object is not needed if you just
  // need some specific FS_* operations. When the FS object is present, it calls
  // into those FS_* methods as needed.
  //
  // In contrast, the old JS FS (libfs.js) does the opposite: it puts all
  // things on the FS object, and copies them to FS_* methods for use from JS
  // library code. Given that the JS FS is implemented entirely in JS, that
  // makes sense there (as almost all that FS object ends up needed anyhow all
  // the time).

  $FS_createDataFile__deps: [
    '$wasmFSPreloadingFlushed', '$wasmFSPreloadedFiles',
    '$FS_create', '$FS_writeFile',
  ],
  $FS_createDataFile: (parent, name, fileData, canRead, canWrite, canOwn) => {
    var pathName = name ? parent + '/' + name : parent;
    var mode = FS_getMode(canRead, canWrite);

    if (!wasmFSPreloadingFlushed) {
      // WasmFS code in the wasm is not ready to be called yet. Cache the
      // files we want to create here in JS, and WasmFS will read them
      // later.
      wasmFSPreloadedFiles.push({pathName, fileData, mode});
    } else {
      // WasmFS is already running, so create the file normally.
      FS_create(pathName, mode);
      FS_writeFile(pathName, fileData);
    }
  },

  $FS_mknod__deps: ['_wasmfs_mknod'],
  $FS_mknod: (path, mode, dev) => FS.handleError(withStackSave(() => {
    var pathBuffer = stringToUTF8OnStack(path);
    return __wasmfs_mknod(pathBuffer, mode, dev);
  })),

  $FS_create__deps: ['$FS_mknod'],
  // Default settings copied from the legacy JS FS API.
  $FS_create: (path, mode = 0o666) => {
    mode &= {{{ cDefs.S_IALLUGO }}};
    mode |= {{{ cDefs.S_IFREG }}};
    return FS_mknod(path, mode, 0);
  },

  $FS_writeFile__deps: ['_wasmfs_write_file', '$stackSave', '$stackRestore', 'malloc', 'free'],
  $FS_writeFile: (path, data) => {
    var sp = stackSave();
    var pathBuffer = stringToUTF8OnStack(path);
    var len = typeof data == 'string' ? lengthBytesUTF8(data) + 1 : data.length;
    var dataBuffer = _malloc(len);
#if ASSERTIONS
    assert(dataBuffer);
#endif
    if (typeof data == 'string') {
      len = stringToUTF8(data, dataBuffer, len);
    } else {
      HEAPU8.set(data, dataBuffer);
    }
    var ret = __wasmfs_write_file(pathBuffer, dataBuffer, len);
    _free(dataBuffer);
    stackRestore(sp);
    return ret;
  },

  $FS_mkdir__deps: ['_wasmfs_mkdir'],
  $FS_mkdir: (path, mode = 0o777) => FS.handleError(withStackSave(() => {
    var buffer = stringToUTF8OnStack(path);
    return __wasmfs_mkdir(buffer, mode);
  })),

  $FS_mkdirTree__docs: `
  /**
   * @param {number=} mode Optionally, the mode to create in. Uses mkdir's
   *                       default if not set.
   */`,
  $FS_mkdirTree__deps: ['$FS_mkdir'],
  $FS_mkdirTree: (path, mode) => {
    var dirs = path.split('/');
    var d = '';
    for (var dir of dirs) {
      if (!dir) continue;
      if (d || PATH.isAbs(path)) d += '/';
      d += dir;
      try {
        FS_mkdir(d, mode);
      } catch(e) {
        if (e.errno != {{{ cDefs.EEXIST }}}) throw e;
      }
    }
  },

  $FS_unlink__deps: ['_wasmfs_unlink'],
  $FS_unlink: (path) => withStackSave(() => {
    var buffer = stringToUTF8OnStack(path);
    return __wasmfs_unlink(buffer);
  }),

  // Wasm access calls.

  _wasmfs_get_num_preloaded_files__deps: [
    '$wasmFSPreloadedFiles',
    '$wasmFSPreloadingFlushed'],
  _wasmfs_get_num_preloaded_files: () => {
    // When this method is called from WasmFS it means that we are about to
    // flush all the preloaded data, so mark that. (There is no call that
    // occurs at the end of that flushing, which would be more natural, but it
    // is fine to mark the flushing here as during the flushing itself no user
    // code can run, so nothing will check whether we have flushed or not.)
    wasmFSPreloadingFlushed = true;
    return wasmFSPreloadedFiles.length;
  },
  _wasmfs_get_num_preloaded_dirs__deps: ['$wasmFSPreloadedDirs'],
  _wasmfs_get_num_preloaded_dirs: () => wasmFSPreloadedDirs.length,
  _wasmfs_get_preloaded_file_mode: (index) => wasmFSPreloadedFiles[index].mode,
  _wasmfs_get_preloaded_parent_path: (index, parentPathBuffer) => {
    var s = wasmFSPreloadedDirs[index].parentPath;
    var len = lengthBytesUTF8(s) + 1;
    stringToUTF8(s, parentPathBuffer, len);
  },
  _wasmfs_get_preloaded_child_path: (index, childNameBuffer) => {
    var s = wasmFSPreloadedDirs[index].childName;
    var len = lengthBytesUTF8(s) + 1;
    stringToUTF8(s, childNameBuffer, len);
  },
  _wasmfs_get_preloaded_path_name__deps: ['$lengthBytesUTF8', '$stringToUTF8'],
  _wasmfs_get_preloaded_path_name: (index, fileNameBuffer) => {
    var s = wasmFSPreloadedFiles[index].pathName;
    var len = lengthBytesUTF8(s) + 1;
    stringToUTF8(s, fileNameBuffer, len);
  },
  _wasmfs_get_preloaded_file_size: (index) =>
    wasmFSPreloadedFiles[index].fileData.length,
  _wasmfs_copy_preloaded_file_data: (index, buffer) =>
    HEAPU8.set(wasmFSPreloadedFiles[index].fileData, buffer),

  _wasmfs_thread_utils_heartbeat__deps: ['emscripten_proxy_execute_queue'],
  _wasmfs_thread_utils_heartbeat: (queue) => {
    var intervalID =
      setInterval(() => {
        if (ABORT) {
          clearInterval(intervalID);
        } else {
          _emscripten_proxy_execute_queue(queue);
        }
      }, 50);
  },

  _wasmfs_stdin_get_char__deps: ['$FS_stdin_getChar'],
  _wasmfs_stdin_get_char: () => {
    // Return the read character, or -1 to indicate EOF.
    var c = FS_stdin_getChar();
    if (typeof c === 'number') {
      return c;
    }
    return -1;
  }
});
