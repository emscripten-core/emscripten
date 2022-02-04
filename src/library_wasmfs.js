var WasmfsLibrary = {
  $wasmFS$preloadedFiles: [],
  $wasmFS$preloadedDirs: [],
  $FS__deps: [
    '$wasmFS$preloadedFiles',
    '$wasmFS$preloadedDirs',
    '$asyncLoad',
#if !MINIMAL_RUNTIME
    // TODO: when preload-plugins are not used, we do not need this.
    '$Browser',
#endif
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
      var dataBuffer = allocate(data);
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

  // JSFile backend

  $wasmFS$JSMemoryFiles : [],
  $wasmFS$JSMemoryFreeList: [],

  _wasmfs_write_js_file__deps: [
    '$wasmFS$JSMemoryFiles',
  ],
  _wasmfs_write_js_file: function(index, buffer, length, offset) {
    try {
      if (!wasmFS$JSMemoryFiles[index]) {
        // Initialize typed array on first write operation.
        wasmFS$JSMemoryFiles[index] = new Uint8Array(offset + length);
      }

      if (offset + length > wasmFS$JSMemoryFiles[index].length) {
        // Resize the typed array if the length of the write buffer exceeds its capacity.
        var oldContents = wasmFS$JSMemoryFiles[index];
        var newContents = new Uint8Array(offset + length);
        newContents.set(oldContents);
        wasmFS$JSMemoryFiles[index] = newContents;
      }

      wasmFS$JSMemoryFiles[index].set(HEAPU8.subarray(buffer, buffer + length), offset);
      return 0;
    } catch (err) {
      return {{{ cDefine('EIO') }}};
    }
  },
  _wasmfs_read_js_file__deps: [
    '$wasmFS$JSMemoryFiles',
  ],
  _wasmfs_read_js_file: function(index, buffer, length, offset) {
    try {
      HEAPU8.set(wasmFS$JSMemoryFiles[index].subarray(offset, offset + length), buffer);
      return 0;
    } catch (err) {
      return {{{ cDefine('EIO') }}};
    }
  },
  _wasmfs_get_js_file_size__deps: [
    '$wasmFS$JSMemoryFiles',
  ],
  _wasmfs_get_js_file_size: function(index) {
    return wasmFS$JSMemoryFiles[index] ? wasmFS$JSMemoryFiles[index].length : 0;
  },
  _wasmfs_create_js_file__deps: [
    '$wasmFS$JSMemoryFiles',
    '$wasmFS$JSMemoryFreeList',
  ],
  _wasmfs_create_js_file: function() {
    // Find a free entry in the $wasmFS$JSMemoryFreeList or append a new entry to
    // wasmFS$JSMemoryFiles.
    if (wasmFS$JSMemoryFreeList.length) {
      // Pop off the top of the free list.
      var index = wasmFS$JSMemoryFreeList.pop();
      return index;
    }
    wasmFS$JSMemoryFiles.push(null);
    return wasmFS$JSMemoryFiles.length - 1;
  },
  _wasmfs_remove_js_file__deps: [
    '$wasmFS$JSMemoryFiles',
    '$wasmFS$JSMemoryFreeList',
  ],
  _wasmfs_remove_js_file: function(index) {
    wasmFS$JSMemoryFiles[index] = null;
    // Add the index to the free list.
    wasmFS$JSMemoryFreeList.push(index);
  },
}

mergeInto(LibraryManager.library, WasmfsLibrary);

if (WASMFS) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$FS');
}
