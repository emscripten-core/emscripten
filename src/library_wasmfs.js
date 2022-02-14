var WasmFSLibrary = {
  $wasmFS$preloadedFiles: [],
  $wasmFS$preloadedDirs: [],
#if USE_CLOSURE_COMPILER
  // Declare variable for Closure, FS.createPreloadedFile() below calls Browser.handledByPreloadPlugin()
  $FS__postset: '/**@suppress {duplicate, undefinedVars}*/var Browser;',
#endif
  $FS__deps: [
    '$wasmFS$preloadedFiles',
    '$wasmFS$preloadedDirs',
    '$asyncLoad',
  ],
  $FS : {
    // TODO: Clean up the following functions - currently copied from library_fs.js directly.
    createPreloadedFile: (parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) => {
      // TODO: use WasmFS code to resolve and join the path here?
      var fullname = name ? parent + '/' + name : parent;
      var dep = getUniqueRunDependency('cp ' + fullname); // might have several active requests for the same fullname
      function processData(byteArray) {
        function finish(byteArray) {
          if (preFinish) preFinish();
          if (!dontCreateFile) {
            FS.createDataFile(parent, name, byteArray, canRead, canWrite, canOwn);
          }
          if (onload) onload();
          removeRunDependency(dep);
        }
#if !MINIMAL_RUNTIME
        if (Browser.handledByPreloadPlugin(byteArray, fullname, finish, () => {
          if (onerror) onerror();
          removeRunDependency(dep);
        })) {
          return;
        }
#endif
        finish(byteArray);
      }
      addRunDependency(dep);
      if (typeof url == 'string') {
        asyncLoad(url, (byteArray) => {
          processData(byteArray);
        }, onerror);
      } else {
        processData(url);
      }
    },
    getMode: (canRead, canWrite) => {
      var mode = 0;
      if (canRead) mode |= {{{ cDefine('S_IRUGO') }}} | {{{ cDefine('S_IXUGO') }}};
      if (canWrite) mode |= {{{ cDefine('S_IWUGO') }}};
      return mode;
    },
    createDataFile: (parent, name, data, canRead, canWrite, canOwn) => {
      // Data files must be cached until the file system itself has been initialized.
      var mode = FS.getMode(canRead, canWrite);
      var pathName = name ? parent + '/' + name : parent;
      wasmFS$preloadedFiles.push({pathName: pathName, fileData: data, mode: mode});
    },
    createPath: (parent, path, canRead, canWrite) => {
      // Cache file path directory names.
      wasmFS$preloadedDirs.push({parentPath: parent, childName: path});
    },
    readFile: (path, opts) => {
      opts = opts || {};
      opts.encoding = opts.encoding || 'binary';
      if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
        throw new Error('Invalid encoding type "' + opts.encoding + '"');
      }

      var pathName = allocateUTF8(path);

      // Copy the file into a JS buffer on the heap.
      var buf = __wasmfs_read_file(pathName);
      // The integer length is returned in the first 8 bytes of the buffer.
      var length = {{{ makeGetValue('buf', '0', 'i64') }}};

      // Default return type is binary.
      // The buffer contents exist 8 bytes after the returned pointer.
      var ret = new Uint8Array(HEAPU8.subarray(buf + 8, buf + 8 + length));
      if (opts.encoding === 'utf8') {
        ret = UTF8ArrayToString(ret, 0);
      }

      _free(pathName);
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
    mkdir: (path) => {
      var buffer = allocateUTF8OnStack(path);
      __wasmfs_mkdir(buffer);
    },
    chdir: (path) => {
      var buffer = allocateUTF8OnStack(path);
      return __wasmfs_chdir(buffer);
    },
    writeFile: (path, data) => {
      var pathBuffer = allocateUTF8OnStack(path);
      var dataBuffer = _malloc(data);
      __wasmfs_write_file(pathBuffer, dataBuffer, data.length);
      _free(dataBuffer);
    },
    symlink: (target, linkpath) => {
      var targetBuffer = allocateUTF8OnStack(target);
      var linkpathBuffer = allocateUTF8OnStack(linkpath);
      __wasmfs_symlink(targetBuffer, linkpathBuffer);
    },
#endif
  },
  _wasmfs_get_num_preloaded_files__deps: ['$wasmFS$preloadedFiles'],
  _wasmfs_get_num_preloaded_files: function() {
    return wasmFS$preloadedFiles.length;
  },
  _wasmfs_get_num_preloaded_dirs__deps: ['$wasmFS$preloadedDirs'],
  _wasmfs_get_num_preloaded_dirs: function() {
    return wasmFS$preloadedDirs.length;
  },
  _wasmfs_get_preloaded_file_mode: function(index) {
    return wasmFS$preloadedFiles[index].mode;
  },
  _wasmfs_get_preloaded_parent_path: function(index, parentPathBuffer) {
    var s = wasmFS$preloadedDirs[index].parentPath;
    var len = lengthBytesUTF8(s) + 1;
    stringToUTF8(s, parentPathBuffer, len);
  },
  _wasmfs_get_preloaded_child_path: function(index, childNameBuffer) {
    var s = wasmFS$preloadedDirs[index].childName;
    var len = lengthBytesUTF8(s) + 1;
    stringToUTF8(s, childNameBuffer, len);
  },
  _wasmfs_get_preloaded_path_name: function(index, fileNameBuffer) {
    var s = wasmFS$preloadedFiles[index].pathName;
    var len = lengthBytesUTF8(s) + 1;
    stringToUTF8(s, fileNameBuffer, len);
  },
  _wasmfs_get_preloaded_file_size: function(index) {
    return wasmFS$preloadedFiles[index].fileData.length;
  },
  _wasmfs_copy_preloaded_file_data: function(index, buffer) {
    HEAPU8.set(wasmFS$preloadedFiles[index].fileData, buffer);
  },

  // Backend support. wasmFS$backends will contain a mapping of backend IDs to
  // the JS code that implements them. This is the JS side of the JSImpl* class
  // in C++, together with the js_impl calls defined right after it.
  $wasmFS$backends: {},

  // JSImpl

  _wasmfs_jsimpl_alloc_file: function(backend, file) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    return wasmFS$backends[backend].allocFile(file);
  },

  _wasmfs_jsimpl_free_file: function(backend, file) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    return wasmFS$backends[backend].freeFile(file);
  },

  _wasmfs_jsimpl_write: function(backend, file, buffer, length, {{{ defineI64Param('offset') }}}) {
    {{{ receiveI64ParamAsDouble('offset') }}}
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    return wasmFS$backends[backend].write(file, buffer, length, offset);
  },

  _wasmfs_jsimpl_read: function(backend, file, buffer, length, {{{ defineI64Param('offset') }}}) {
    {{{ receiveI64ParamAsDouble('offset') }}}
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    return wasmFS$backends[backend].read(file, buffer, length, offset);
  },

  _wasmfs_jsimpl_get_size: function(backend, file) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    return wasmFS$backends[backend].getSize(file);
  },

  // ProxiedAsyncJSImpl. Each function receives a function pointer and a
  // parameter. We convert those into a convenient Promise API for the
  // implementors of backends: the hooks we call should return Promises, which
  // we then connect to the calling C++.

  // TODO: arg is void*, which for MEMORY64 will be 64-bit. we need a way to
  //       declare arg in the function signature here (like defineI64Param,
  //       but that varies for wasm32/wasm64), and a way to do makeDynCall that
  //       adds a 'p' signature type for pointer, or something like that
  //       (however, dyncalls might also just work, given in MEMORY64 we assume
  //       WASM_BIGINT so the pointer is just a single argument, just like in
  //       wasm32).
  _wasmfs_jsimpl_async_alloc_file: async function(backend, file, fptr, arg) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    {{{ runtimeKeepalivePush() }}}
    await wasmFS$backends[backend].allocFile(file);
    {{{ runtimeKeepalivePop() }}}
    {{{ makeDynCall('vi', 'fptr') }}}(arg);
  },

  _wasmfs_jsimpl_async_free_file: async function(backend, file, fptr, arg) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    {{{ runtimeKeepalivePush() }}}
    await wasmFS$backends[backend].freeFile(file);
    {{{ runtimeKeepalivePop() }}}
    {{{ makeDynCall('vi', 'fptr') }}}(arg);
  },

  _wasmfs_jsimpl_async_write: async function(backend, file, buffer, length, {{{ defineI64Param('offset') }}}, fptr, arg) {
    {{{ receiveI64ParamAsDouble('offset') }}}
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    {{{ runtimeKeepalivePush() }}}
    var size = await wasmFS$backends[backend].write(file, buffer, length, offset);
    {{{ runtimeKeepalivePop() }}}
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.result, '0', 'i32') }}};
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.offset, 'size', 'i64') }}};
    {{{ makeDynCall('vi', 'fptr') }}}(arg);
  },

  _wasmfs_jsimpl_async_read: async function(backend, file, buffer, length, {{{ defineI64Param('offset') }}}, fptr, arg) {
    {{{ receiveI64ParamAsDouble('offset') }}}
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    {{{ runtimeKeepalivePush() }}}
    var size = await wasmFS$backends[backend].read(file, buffer, length, offset);
    {{{ runtimeKeepalivePop() }}}
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.result, '0', 'i32') }}};
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.offset, 'size', 'i64') }}};
    {{{ makeDynCall('vi', 'fptr') }}}(arg);
  },

  _wasmfs_jsimpl_async_get_size: async function(backend, file, fptr, arg) {
#if ASSERTIONS
    assert(wasmFS$backends[backend]);
#endif
    {{{ runtimeKeepalivePush() }}}
    var size = await wasmFS$backends[backend].getSize(file);
    {{{ runtimeKeepalivePop() }}}
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.result, '0', 'i32') }}};
    {{{ makeSetValue('arg', C_STRUCTS.CallbackState.offset, 'size', 'i64') }}};
    {{{ makeDynCall('vi', 'fptr') }}}(arg);
  },
}

mergeInto(LibraryManager.library, WasmFSLibrary);

DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$FS');
