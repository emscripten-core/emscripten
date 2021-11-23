var WasmfsLibrary = {
  $wasmFS$preloadedFiles: [],
  $wasmFS$preloadedDirs: [],
  $FS__deps: ['$wasmFS$preloadedFiles', '$wasmFS$preloadedDirs'],
  $FS : {
    // TODO: Clean up the following functions - currently copied from library_fs.js directly.
    createPreloadedFile: function(parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) {
        Browser.init(); 
        var fullname = name ? PATH_FS.resolve(PATH.join2(parent, name)) : parent;
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
          var handled = false;
          Module['preloadPlugins'].forEach(function(plugin) {
            if (handled) return;
            if (plugin['canHandle'](fullname)) {
              plugin['handle'](byteArray, fullname, finish, function() {
                if (onerror) onerror();
                removeRunDependency(dep);
              });
              handled = true;
            }
          });
          if (!handled) finish(byteArray);
        }
        addRunDependency(dep);
        if (typeof url == 'string') {
          asyncLoad(url, function(byteArray) {
            processData(byteArray);
          }, onerror);
        } else {
          processData(url);
        }
      },
    getMode: function(canRead, canWrite) {
      var mode = 0;
      if (canRead) mode |= {{{ cDefine('S_IRUGO') }}} | {{{ cDefine('S_IXUGO') }}};
      if (canWrite) mode |= {{{ cDefine('S_IWUGO') }}};
      return mode;
    },
    createDataFile: function(parent, name, data, canRead, canWrite, canOwn) {
      // Data files must be cached until the file system itself has been initialized.
      var mode = FS.getMode(canRead, canWrite);
      wasmFS$preloadedFiles.push({pathName: parent, fileData: data, mode: mode});
    },
    createPath: function(parent, path, canRead, canWrite) {
      // Cache file path directory names.
      wasmFS$preloadedDirs.push({parentPath: parent, childName: path});
    },
    readFile: function(path, opts) {
      opts = opts || {};
      opts.encoding = opts.encoding || 'binary';
      if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
        throw new Error('Invalid encoding type "' + opts.encoding + '"');
      }

      var pathName = allocateUTF8(path);
      
      // Copy the file into a JS buffer on the heap.
      var buf = _emscripten_wasmfs_read_file(pathName);
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
    cwd: function() {
      // TODO: Remove dependency on FS.cwd().
      // User code should not be using FS.cwd().
      // For file preloading, cwd should be '/' to begin with.
      return '/';
    }
  }
}

mergeInto(LibraryManager.library, WasmfsLibrary);

if (WASMFS) {
  DEFAULT_LIBRARY_FUNCS_TO_INCLUDE.push('$FS');
}
