//"use strict";

// An implementation of a libc for the web. Basically, implementations of
// the various standard C libraries, that can be called from compiled code,
// and work using the actual JavaScript environment.
//
// We search the Library object when there is an external function. If the
// entry in the Library is a function, we insert it. If it is a string, we
// do another lookup in the library (a simple way to write a function once,
// if it can be called by different names). We also allow dependencies,
// using __deps. Initialization code to be run after allocating all
// global constants can be defined by __postset.
//
// Note that the full function name will be '_' + the name in the Library
// object. For convenience, the short name appears here. Note that if you add a
// new function with an '_', it will not be found.

LibraryManager.library = {
  // ==========================================================================
  // File system base.
  // ==========================================================================

  stdin: 0,
  stdout: 0,
  stderr: 0,
  _impure_ptr: 0,

  $FS__deps: ['$ERRNO_CODES', '__setErrNo', 'stdin', 'stdout', 'stderr', '_impure_ptr'],
  $FS__postset: '__ATINIT__.unshift({ func: function() { if (!Module["noFSInit"] && !FS.init.initialized) FS.init() } });' +
                '__ATMAIN__.push({ func: function() { FS.ignorePermissions = false } });' +
                '__ATEXIT__.push({ func: function() { FS.quit() } });' +
                // export some names through closure
                'Module["FS_createFolder"] = FS.createFolder;' +
                'Module["FS_createPath"] = FS.createPath;' +
                'Module["FS_createDataFile"] = FS.createDataFile;' +
                'Module["FS_createPreloadedFile"] = FS.createPreloadedFile;' +
                'Module["FS_createLazyFile"] = FS.createLazyFile;' +
                'Module["FS_createLink"] = FS.createLink;' +
                'Module["FS_createDevice"] = FS.createDevice;',
  $FS: {
    // The path to the current folder.
    currentPath: '/',
    // The inode to assign to the next created object.
    nextInode: 2,
    // Currently opened file or directory streams. Padded with null so the zero
    // index is unused, as the indices are used as pointers. This is not split
    // into separate fileStreams and folderStreams lists because the pointers
    // must be interchangeable, e.g. when used in fdopen().
    // streams is kept as a dense array. It may contain |null| to fill in
    // holes, however.
    streams: [null],
#if ASSERTIONS
    checkStreams: function() {
      for (var i in FS.streams) assert(i >= 0 && i < FS.streams.length); // no keys not in dense span
      for (var i = 0; i < FS.streams.length; i++) assert(typeof FS.streams[i] == 'object'); // no non-null holes in dense span
    },
#endif
    // Whether we are currently ignoring permissions. Useful when preparing the
    // filesystem and creating files inside read-only folders.
    // This is set to false when the runtime is initialized, allowing you
    // to modify the filesystem freely before run() is called.
    ignorePermissions: true,
    joinPath: function(parts, forceRelative) {
      var ret = parts[0];
      for (var i = 1; i < parts.length; i++) {
        if (ret[ret.length-1] != '/') ret += '/';
        ret += parts[i];
      }
      if (forceRelative && ret[0] == '/') ret = ret.substr(1);
      return ret;
    },
    // Converts any path to an absolute path. Resolves embedded "." and ".."
    // parts.
    absolutePath: function(relative, base) {
      if (typeof relative !== 'string') return null;
      if (base === undefined) base = FS.currentPath;
      if (relative && relative[0] == '/') base = '';
      var full = base + '/' + relative;
      var parts = full.split('/').reverse();
      var absolute = [''];
      while (parts.length) {
        var part = parts.pop();
        if (part == '' || part == '.') {
          // Nothing.
        } else if (part == '..') {
          if (absolute.length > 1) absolute.pop();
        } else {
          absolute.push(part);
        }
      }
      return absolute.length == 1 ? '/' : absolute.join('/');
    },
    // Analyzes a relative or absolute path returning a description, containing:
    //   isRoot: Whether the path points to the root.
    //   exists: Whether the object at the path exists.
    //   error: If !exists, this will contain the errno code of the cause.
    //   name: The base name of the object (null if !parentExists).
    //   path: The absolute path to the object, with all links resolved.
    //   object: The filesystem record of the object referenced by the path.
    //   parentExists: Whether the parent of the object exist and is a folder.
    //   parentPath: The absolute path to the parent folder.
    //   parentObject: The filesystem record of the parent folder.
    analyzePath: function(path, dontResolveLastLink, linksVisited) {
      var ret = {
        isRoot: false,
        exists: false,
        error: 0,
        name: null,
        path: null,
        object: null,
        parentExists: false,
        parentPath: null,
        parentObject: null
      };
#if FS_LOG
      var inputPath = path;
      function log() {
        Module['print']('FS.analyzePath("' + inputPath + '", ' +
                                             dontResolveLastLink + ', ' +
                                             linksVisited + ') => {' +
                        'isRoot: ' + ret.isRoot + ', ' +
                        'exists: ' + ret.exists + ', ' +
                        'error: ' + ret.error + ', ' +
                        'name: "' + ret.name + '", ' +
                        'path: "' + ret.path + '", ' +
                        'object: ' + ret.object + ', ' +
                        'parentExists: ' + ret.parentExists + ', ' +
                        'parentPath: "' + ret.parentPath + '", ' +
                        'parentObject: ' + ret.parentObject + '}');
      }
#endif
      path = FS.absolutePath(path);
      if (path == '/') {
        ret.isRoot = true;
        ret.exists = ret.parentExists = true;
        ret.name = '/';
        ret.path = ret.parentPath = '/';
        ret.object = ret.parentObject = FS.root;
      } else if (path !== null) {
        linksVisited = linksVisited || 0;
        path = path.slice(1).split('/');
        var current = FS.root;
        var traversed = [''];
        while (path.length) {
          if (path.length == 1 && current.isFolder) {
            ret.parentExists = true;
            ret.parentPath = traversed.length == 1 ? '/' : traversed.join('/');
            ret.parentObject = current;
            ret.name = path[0];
          }
          var target = path.shift();
          if (!current.isFolder) {
            ret.error = ERRNO_CODES.ENOTDIR;
            break;
          } else if (!current.read) {
            ret.error = ERRNO_CODES.EACCES;
            break;
          } else if (!current.contents.hasOwnProperty(target)) {
            ret.error = ERRNO_CODES.ENOENT;
            break;
          }
          current = current.contents[target];
          if (current.link && !(dontResolveLastLink && path.length == 0)) {
            if (linksVisited > 40) { // Usual Linux SYMLOOP_MAX.
              ret.error = ERRNO_CODES.ELOOP;
              break;
            }
            var link = FS.absolutePath(current.link, traversed.join('/'));
            ret = FS.analyzePath([link].concat(path).join('/'),
                                 dontResolveLastLink, linksVisited + 1);
#if FS_LOG
            log();
#endif
            return ret;
          }
          traversed.push(target);
          if (path.length == 0) {
            ret.exists = true;
            ret.path = traversed.join('/');
            ret.object = current;
          }
        }
      }
#if FS_LOG
      log();
#endif
      return ret;
    },
    // Finds the file system object at a given path. If dontResolveLastLink is
    // set to true and the object is a symbolic link, it will be returned as is
    // instead of being resolved. Links embedded in the path are still resolved.
    findObject: function(path, dontResolveLastLink) {
      FS.ensureRoot();
      var ret = FS.analyzePath(path, dontResolveLastLink);
      if (ret.exists) {
        return ret.object;
      } else {
        ___setErrNo(ret.error);
        return null;
      }
    },
    // Creates a file system record: file, link, device or folder.
    createObject: function(parent, name, properties, canRead, canWrite) {
#if FS_LOG
      Module['print']('FS.createObject("' + parent + '", ' +
                                      '"' + name + '", ' +
                                          JSON.stringify(properties) + ', ' +
                                          canRead + ', ' +
                                          canWrite + ')');
#endif
      if (!parent) parent = '/';
      if (typeof parent === 'string') parent = FS.findObject(parent);

      if (!parent) {
        ___setErrNo(ERRNO_CODES.EACCES);
        throw new Error('Parent path must exist.');
      }
      if (!parent.isFolder) {
        ___setErrNo(ERRNO_CODES.ENOTDIR);
        throw new Error('Parent must be a folder.');
      }
      if (!parent.write && !FS.ignorePermissions) {
        ___setErrNo(ERRNO_CODES.EACCES);
        throw new Error('Parent folder must be writeable.');
      }
      if (!name || name == '.' || name == '..') {
        ___setErrNo(ERRNO_CODES.ENOENT);
        throw new Error('Name must not be empty.');
      }
      if (parent.contents.hasOwnProperty(name)) {
        ___setErrNo(ERRNO_CODES.EEXIST);
        throw new Error("Can't overwrite object.");
      }

      parent.contents[name] = {
        read: canRead === undefined ? true : canRead,
        write: canWrite === undefined ? false : canWrite,
        timestamp: Date.now(),
        inodeNumber: FS.nextInode++
      };
      for (var key in properties) {
        if (properties.hasOwnProperty(key)) {
          parent.contents[name][key] = properties[key];
        }
      }

      return parent.contents[name];
    },
    // Creates a folder.
    createFolder: function(parent, name, canRead, canWrite) {
      var properties = {isFolder: true, isDevice: false, contents: {}};
      return FS.createObject(parent, name, properties, canRead, canWrite);
    },
    // Creates a a folder and all its missing parents.
    createPath: function(parent, path, canRead, canWrite) {
      var current = FS.findObject(parent);
      if (current === null) throw new Error('Invalid parent.');
      path = path.split('/').reverse();
      while (path.length) {
        var part = path.pop();
        if (!part) continue;
        if (!current.contents.hasOwnProperty(part)) {
          FS.createFolder(current, part, canRead, canWrite);
        }
        current = current.contents[part];
      }
      return current;
    },

    // Creates a file record, given specific properties.
    createFile: function(parent, name, properties, canRead, canWrite) {
      properties.isFolder = false;
      return FS.createObject(parent, name, properties, canRead, canWrite);
    },
    // Creates a file record from existing data.
    createDataFile: function(parent, name, data, canRead, canWrite) {
      if (typeof data === 'string') {
        var dataArray = new Array(data.length);
        for (var i = 0, len = data.length; i < len; ++i) dataArray[i] = data.charCodeAt(i);
        data = dataArray;
      }
      var properties = {
        isDevice: false,
        contents: data.subarray ? data.subarray(0) : data // as an optimization, create a new array wrapper (not buffer) here, to help JS engines understand this object
      };
      return FS.createFile(parent, name, properties, canRead, canWrite);
    },
    // Creates a file record for lazy-loading from a URL. XXX This requires a synchronous
    // XHR, which is not possible in browsers except in a web worker! Use preloading,
    // either --preload-file in emcc or FS.createPreloadedFile
    createLazyFile: function(parent, name, url, canRead, canWrite) {

      if (typeof XMLHttpRequest !== 'undefined') {
        if (!ENVIRONMENT_IS_WORKER) throw 'Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc';
        // Lazy chunked Uint8Array (implements get and length from Uint8Array). Actual getting is abstracted away for eventual reuse.
        var LazyUint8Array = function(chunkSize, length) {
          this.length = length;
          this.chunkSize = chunkSize;
          this.chunks = []; // Loaded chunks. Index is the chunk number
        }
        LazyUint8Array.prototype.get = function(idx) {
          if (idx > this.length-1 || idx < 0) {
            return undefined;
          }
          var chunkOffset = idx % chunkSize;
          var chunkNum = Math.floor(idx / chunkSize);
          return this.getter(chunkNum)[chunkOffset];
        }
        LazyUint8Array.prototype.setDataGetter = function(getter) {
          this.getter = getter;
        }
  
        // Find length
        var xhr = new XMLHttpRequest();
        xhr.open('HEAD', url, false);
        xhr.send(null);
        if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
        var datalength = Number(xhr.getResponseHeader("Content-length"));
        var header;
        var hasByteServing = (header = xhr.getResponseHeader("Accept-Ranges")) && header === "bytes";
#if SMALL_XHR_CHUNKS
        var chunkSize = 1024; // Chunk size in bytes
#else
        var chunkSize = 1024*1024; // Chunk size in bytes
#endif
        if (!hasByteServing) chunkSize = datalength;
  
        // Function to get a range from the remote URL.
        var doXHR = (function(from, to) {
          if (from > to) throw new Error("invalid range (" + from + ", " + to + ") or no bytes requested!");
          if (to > datalength-1) throw new Error("only " + datalength + " bytes available! programmer error!");
  
          // TODO: Use mozResponseArrayBuffer, responseStream, etc. if available.
          var xhr = new XMLHttpRequest();
          xhr.open('GET', url, false);
          if (datalength !== chunkSize) xhr.setRequestHeader("Range", "bytes=" + from + "-" + to);
  
          // Some hints to the browser that we want binary data.
          if (typeof Uint8Array != 'undefined') xhr.responseType = 'arraybuffer';
          if (xhr.overrideMimeType) {
            xhr.overrideMimeType('text/plain; charset=x-user-defined');
          }
  
          xhr.send(null);
          if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
          if (xhr.response !== undefined) {
            return new Uint8Array(xhr.response || []);
          } else {
            return intArrayFromString(xhr.responseText || '', true);
          }
        });
  
        var lazyArray = new LazyUint8Array(chunkSize, datalength);
        lazyArray.setDataGetter(function(chunkNum) {
          var start = chunkNum * lazyArray.chunkSize;
          var end = (chunkNum+1) * lazyArray.chunkSize - 1; // including this byte
          end = Math.min(end, datalength-1); // if datalength-1 is selected, this is the last block
          if (typeof(lazyArray.chunks[chunkNum]) === "undefined") {
            lazyArray.chunks[chunkNum] = doXHR(start, end);
          }
          if (typeof(lazyArray.chunks[chunkNum]) === "undefined") throw new Error("doXHR failed!");
          return lazyArray.chunks[chunkNum];
        });
        var properties = { isDevice: false, contents: lazyArray };
      } else {
        var properties = { isDevice: false, url: url };
      }

      return FS.createFile(parent, name, properties, canRead, canWrite);
    },
    // Preloads a file asynchronously. You can call this before run, for example in
    // preRun. run will be delayed until this file arrives and is set up.
    // If you call it after run(), you may want to pause the main loop until it
    // completes, if so, you can use the onload parameter to be notified when
    // that happens.
    // In addition to normally creating the file, we also asynchronously preload
    // the browser-friendly versions of it: For an image, we preload an Image
    // element and for an audio, and Audio. These are necessary for SDL_Image
    // and _Mixer to find the files in preloadedImages/Audios.
    // You can also call this with a typed array instead of a url. It will then
    // do preloading for the Image/Audio part, as if the typed array were the
    // result of an XHR that you did manually.
    createPreloadedFile: function(parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile) {
      Browser.ensureObjects();
      var fullname = FS.joinPath([parent, name], true);
      function processData(byteArray) {
        function finish(byteArray) {
          if (!dontCreateFile) {
            FS.createDataFile(parent, name, byteArray, canRead, canWrite);
          }
          if (onload) onload();
          removeRunDependency('cp ' + fullname);
        }
        var handled = false;
        Module['preloadPlugins'].forEach(function(plugin) {
          if (handled) return;
          if (plugin['canHandle'](fullname)) {
            plugin['handle'](byteArray, fullname, finish, function() {
              if (onerror) onerror();
              removeRunDependency('cp ' + fullname);
            });
            handled = true;
          }
        });
        if (!handled) finish(byteArray);
      }
      addRunDependency('cp ' + fullname);
      if (typeof url == 'string') {
        Browser.asyncLoad(url, function(byteArray) {
          processData(byteArray);
        }, onerror);
      } else {
        processData(url);
      }
    },
    // Creates a link to a sepcific local path.
    createLink: function(parent, name, target, canRead, canWrite) {
      var properties = {isDevice: false, link: target};
      return FS.createFile(parent, name, properties, canRead, canWrite);
    },
    // Creates a character device with input and output callbacks:
    //   input: Takes no parameters, returns a byte value or null if no data is
    //          currently available.
    //   output: Takes a byte value; doesn't return anything. Can also be passed
    //           null to perform a flush of any cached data.
    createDevice: function(parent, name, input, output) {
      if (!(input || output)) {
        throw new Error('A device must have at least one callback defined.');
      }
      var ops = {isDevice: true, input: input, output: output};
      return FS.createFile(parent, name, ops, Boolean(input), Boolean(output));
    },
    // Makes sure a file's contents are loaded. Returns whether the file has
    // been loaded successfully. No-op for files that have been loaded already.
    forceLoadFile: function(obj) {
      if (obj.isDevice || obj.isFolder || obj.link || obj.contents) return true;
      var success = true;
      if (typeof XMLHttpRequest !== 'undefined') {
        throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");
      } else if (Module['read']) {
        // Command-line.
        try {
          // WARNING: Can't read binary files in V8's d8 or tracemonkey's js, as
          //          read() will try to parse UTF8.
          obj.contents = intArrayFromString(Module['read'](obj.url), true);
        } catch (e) {
          success = false;
        }
      } else {
        throw new Error('Cannot load without read() or XMLHttpRequest.');
      }
      if (!success) ___setErrNo(ERRNO_CODES.EIO);
      return success;
    },
    ensureRoot: function() {
      if (FS.root) return;
      // The main file system tree. All the contents are inside this.
      FS.root = {
        read: true,
        write: true,
        isFolder: true,
        isDevice: false,
        timestamp: Date.now(),
        inodeNumber: 1,
        contents: {}
      };
    },
    // Initializes the filesystems with stdin/stdout/stderr devices, given
    // optional handlers.
    init: function(input, output, error) {
      // Make sure we initialize only once.
      assert(!FS.init.initialized, 'FS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)');
      FS.init.initialized = true;

      FS.ensureRoot();

      // Allow Module.stdin etc. to provide defaults, if none explicitly passed to us here
      input = input || Module['stdin'];
      output = output || Module['stdout'];
      error = error || Module['stderr'];

      // Default handlers.
      var stdinOverridden = true, stdoutOverridden = true, stderrOverridden = true;
      if (!input) {
        stdinOverridden = false;
        input = function() {
          if (!input.cache || !input.cache.length) {
            var result;
            if (typeof window != 'undefined' &&
                typeof window.prompt == 'function') {
              // Browser.
              result = window.prompt('Input: ');
              if (result === null) result = String.fromCharCode(0); // cancel ==> EOF
            } else if (typeof readline == 'function') {
              // Command line.
              result = readline();
            }
            if (!result) result = '';
            input.cache = intArrayFromString(result + '\n', true);
          }
          return input.cache.shift();
        };
      }
      var utf8 = new Runtime.UTF8Processor();
      function simpleOutput(val) {
        if (val === null || val === '\n'.charCodeAt(0)) {
          output.printer(output.buffer.join(''));
          output.buffer = [];
        } else {
          output.buffer.push(utf8.processCChar(val));
        }
      }
      if (!output) {
        stdoutOverridden = false;
        output = simpleOutput;
      }
      if (!output.printer) output.printer = Module['print'];
      if (!output.buffer) output.buffer = [];
      if (!error) {
        stderrOverridden = false;
        error = simpleOutput;
      }
      if (!error.printer) error.printer = Module['print'];
      if (!error.buffer) error.buffer = [];

      // Create the temporary folder, if not already created
      try {
        FS.createFolder('/', 'tmp', true, true);
      } catch(e) {}

      // Create the I/O devices.
      var devFolder = FS.createFolder('/', 'dev', true, true);
      var stdin = FS.createDevice(devFolder, 'stdin', input);
      var stdout = FS.createDevice(devFolder, 'stdout', null, output);
      var stderr = FS.createDevice(devFolder, 'stderr', null, error);
      FS.createDevice(devFolder, 'tty', input, output);

      // Create default streams.
      FS.streams[1] = {
        path: '/dev/stdin',
        object: stdin,
        position: 0,
        isRead: true,
        isWrite: false,
        isAppend: false,
        isTerminal: !stdinOverridden,
        error: false,
        eof: false,
        ungotten: []
      };
      FS.streams[2] = {
        path: '/dev/stdout',
        object: stdout,
        position: 0,
        isRead: false,
        isWrite: true,
        isAppend: false,
        isTerminal: !stdoutOverridden,
        error: false,
        eof: false,
        ungotten: []
      };
      FS.streams[3] = {
        path: '/dev/stderr',
        object: stderr,
        position: 0,
        isRead: false,
        isWrite: true,
        isAppend: false,
        isTerminal: !stderrOverridden,
        error: false,
        eof: false,
        ungotten: []
      };
      // Allocate these on the stack (and never free, we are called from ATINIT or earlier), to keep their locations low
      _stdin = allocate([1], 'void*', ALLOC_STACK);
      _stdout = allocate([2], 'void*', ALLOC_STACK);
      _stderr = allocate([3], 'void*', ALLOC_STACK);

      // Other system paths
      FS.createPath('/', 'dev/shm/tmp', true, true); // temp files

      // Newlib initialization
      for (var i = FS.streams.length; i < Math.max(_stdin, _stdout, _stderr) + {{{ QUANTUM_SIZE }}}; i++) {
        FS.streams[i] = null; // Make sure to keep FS.streams dense
      }
      FS.streams[_stdin] = FS.streams[1];
      FS.streams[_stdout] = FS.streams[2];
      FS.streams[_stderr] = FS.streams[3];
#if ASSERTIONS
      FS.checkStreams();
      assert(FS.streams.length < 1024); // at this early stage, we should not have a large set of file descriptors - just a few
#endif
      __impure_ptr = allocate([ allocate(
        {{{ Runtime.QUANTUM_SIZE === 4 ? '[0, 0, 0, 0, _stdin, 0, 0, 0, _stdout, 0, 0, 0, _stderr, 0, 0, 0]' : '[0, _stdin, _stdout, _stderr]' }}},
        'void*', ALLOC_STATIC) ], 'void*', ALLOC_STATIC);
    },

    quit: function() {
      if (!FS.init.initialized) return;
      // Flush any partially-printed lines in stdout and stderr. Careful, they may have been closed
      if (FS.streams[2] && FS.streams[2].object.output.buffer.length > 0) FS.streams[2].object.output('\n'.charCodeAt(0));
      if (FS.streams[3] && FS.streams[3].object.output.buffer.length > 0) FS.streams[3].object.output('\n'.charCodeAt(0));
    },

    // Standardizes a path. Useful for making comparisons of pathnames work in a consistent manner.
    // For example, ./file and file are really the same, so this function will remove ./
    standardizePath: function(path) {
      if (path.substr(0, 2) == './') path = path.substr(2);
      return path;
    },

    deleteFile: function(path) {
      path = FS.analyzePath(path);
      if (!path.parentExists || !path.exists) {
        throw 'Invalid path ' + path;
      }
      delete path.parentObject.contents[path.name];
    }
  },

  // ==========================================================================
  // dirent.h
  // ==========================================================================

  __dirent_struct_layout: Runtime.generateStructInfo(['d_ino', 'd_name', 'd_off', 'd_reclen', 'd_type'], '%struct.dirent'),
  opendir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', '__dirent_struct_layout'],
  opendir: function(dirname) {
    // DIR *opendir(const char *dirname);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/opendir.html
    // NOTE: Calculating absolute path redundantly since we need to associate it
    //       with the opened stream.
    var path = FS.absolutePath(Pointer_stringify(dirname));
    if (path === null) {
      ___setErrNo(ERRNO_CODES.ENOENT);
      return 0;
    }
    var target = FS.findObject(path);
    if (target === null) return 0;
    if (!target.isFolder) {
      ___setErrNo(ERRNO_CODES.ENOTDIR);
      return 0;
    } else if (!target.read) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return 0;
    }
    var id = FS.streams.length; // Keep dense
    var contents = [];
    for (var key in target.contents) contents.push(key);
    FS.streams[id] = {
      path: path,
      object: target,
      // An index into contents. Special values: -2 is ".", -1 is "..".
      position: -2,
      isRead: true,
      isWrite: false,
      isAppend: false,
      error: false,
      eof: false,
      ungotten: [],
      // Folder-specific properties:
      // Remember the contents at the time of opening in an array, so we can
      // seek between them relying on a single order.
      contents: contents,
      // Each stream has its own area for readdir() returns.
      currentEntry: _malloc(___dirent_struct_layout.__size__)
    };
#if ASSERTIONS
    FS.checkStreams();
#endif
    return id;
  },
  closedir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  closedir: function(dirp) {
    // int closedir(DIR *dirp);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/closedir.html
    if (!FS.streams[dirp] || !FS.streams[dirp].object.isFolder) {
      return ___setErrNo(ERRNO_CODES.EBADF);
    } else {
      _free(FS.streams[dirp].currentEntry);
      FS.streams[dirp] = null;
      return 0;
    }
  },
  telldir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  telldir: function(dirp) {
    // long int telldir(DIR *dirp);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/telldir.html
    if (!FS.streams[dirp] || !FS.streams[dirp].object.isFolder) {
      return ___setErrNo(ERRNO_CODES.EBADF);
    } else {
      return FS.streams[dirp].position;
    }
  },
  seekdir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  seekdir: function(dirp, loc) {
    // void seekdir(DIR *dirp, long int loc);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/seekdir.html
    if (!FS.streams[dirp] || !FS.streams[dirp].object.isFolder) {
      ___setErrNo(ERRNO_CODES.EBADF);
    } else {
      var entries = 0;
      for (var key in FS.streams[dirp].contents) entries++;
      if (loc >= entries) {
        ___setErrNo(ERRNO_CODES.EINVAL);
      } else {
        FS.streams[dirp].position = loc;
      }
    }
  },
  rewinddir__deps: ['seekdir'],
  rewinddir: function(dirp) {
    // void rewinddir(DIR *dirp);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/rewinddir.html
    _seekdir(dirp, -2);
  },
  readdir_r__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', '__dirent_struct_layout'],
  readdir_r: function(dirp, entry, result) {
    // int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/readdir_r.html
    if (!FS.streams[dirp] || !FS.streams[dirp].object.isFolder) {
      return ___setErrNo(ERRNO_CODES.EBADF);
    }
    var stream = FS.streams[dirp];
    var loc = stream.position;
    var entries = 0;
    for (var key in stream.contents) entries++;
    if (loc < -2 || loc >= entries) {
      {{{ makeSetValue('result', '0', '0', 'i8*') }}}
    } else {
      var name, inode, type;
      if (loc === -2) {
        name = '.';
        inode = 1;  // Really undefined.
        type = 4; //DT_DIR
      } else if (loc === -1) {
        name = '..';
        inode = 1;  // Really undefined.
        type = 4; //DT_DIR
      } else {
        var object;
        name = stream.contents[loc];
        object = stream.object.contents[name];
        inode = object.inodeNumber;
        type = object.isDevice ? 2 // DT_CHR, character device.
              : object.isFolder ? 4 // DT_DIR, directory.
              : object.link !== undefined ? 10 // DT_LNK, symbolic link.
              : 8; // DT_REG, regular file.
      }
      stream.position++;
      var offsets = ___dirent_struct_layout;
      {{{ makeSetValue('entry', 'offsets.d_ino', 'inode', 'i32') }}}
      {{{ makeSetValue('entry', 'offsets.d_off', 'stream.position', 'i32') }}}
      {{{ makeSetValue('entry', 'offsets.d_reclen', 'name.length + 1', 'i32') }}}
      for (var i = 0; i < name.length; i++) {
        {{{ makeSetValue('entry + offsets.d_name', 'i', 'name.charCodeAt(i)', 'i8') }}}
      }
      {{{ makeSetValue('entry + offsets.d_name', 'i', '0', 'i8') }}}
      {{{ makeSetValue('entry', 'offsets.d_type', 'type', 'i8') }}}
      {{{ makeSetValue('result', '0', 'entry', 'i8*') }}}
    }
    return 0;
  },
  readdir__deps: ['readdir_r', '__setErrNo', '$ERRNO_CODES'],
  readdir: function(dirp) {
    // struct dirent *readdir(DIR *dirp);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/readdir_r.html
    if (!FS.streams[dirp] || !FS.streams[dirp].object.isFolder) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return 0;
    } else {
      if (!_readdir.result) _readdir.result = _malloc(4);
      _readdir_r(dirp, FS.streams[dirp].currentEntry, _readdir.result);
      if ({{{ makeGetValue(0, '_readdir.result', 'i8*') }}} === 0) {
        return 0;
      } else {
        return FS.streams[dirp].currentEntry;
      }
    }
  },
  __01readdir64_: 'readdir',
  // TODO: Check if we need to link any other aliases.

  // ==========================================================================
  // utime.h
  // ==========================================================================

  __utimbuf_struct_layout: Runtime.generateStructInfo(['actime', 'modtime'], '%struct.utimbuf'),
  utime__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', '__utimbuf_struct_layout'],
  utime: function(path, times) {
    // int utime(const char *path, const struct utimbuf *times);
    // http://pubs.opengroup.org/onlinepubs/009695399/basedefs/utime.h.html
    var time;
    if (times) {
      // NOTE: We don't keep track of access timestamps.
      var offset = ___utimbuf_struct_layout.modtime;
      time = {{{ makeGetValue('times', 'offset', 'i32') }}}
      time *= 1000;
    } else {
      time = Date.now();
    }
    var file = FS.findObject(Pointer_stringify(path));
    if (file === null) return -1;
    if (!file.write) {
      ___setErrNo(ERRNO_CODES.EPERM);
      return -1;
    }
    file.timestamp = time;
    return 0;
  },

  // ==========================================================================
  // libgen.h
  // ==========================================================================

  __libgenSplitName: function(path) {
    if (path === 0 || {{{ makeGetValue('path', 0, 'i8') }}} === 0) {
      // Null or empty results in '.'.
      var me = ___libgenSplitName;
      if (!me.ret) {
        me.ret = allocate(['.'.charCodeAt(0), 0], 'i8', ALLOC_NORMAL);
      }
      return [me.ret, -1];
    } else {
      var slash = '/'.charCodeAt(0);
      var allSlashes = true;
      var slashPositions = [];
      for (var i = 0; {{{ makeGetValue('path', 'i', 'i8') }}} !== 0; i++) {
        if ({{{ makeGetValue('path', 'i', 'i8') }}} === slash) {
          slashPositions.push(i);
        } else {
          allSlashes = false;
        }
      }
      var length = i;
      if (allSlashes) {
        // All slashes result in a single slash.
        {{{ makeSetValue('path', '1', '0', 'i8') }}}
        return [path, -1];
      } else {
        // Strip trailing slashes.
        while (slashPositions.length &&
               slashPositions[slashPositions.length - 1] == length - 1) {
          {{{ makeSetValue('path', 'slashPositions.pop(i)', '0', 'i8') }}}
          length--;
        }
        return [path, slashPositions.pop()];
      }
    }
  },
  basename__deps: ['__libgenSplitName'],
  basename: function(path) {
    // char *basename(char *path);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/basename.html
    var result = ___libgenSplitName(path);
    return result[0] + result[1] + 1;
  },
  __xpg_basename: 'basename',
  dirname__deps: ['__libgenSplitName'],
  dirname: function(path) {
    // char *dirname(char *path);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/dirname.html
    var result = ___libgenSplitName(path);
    if (result[1] == 0) {
      {{{ makeSetValue('result[0]', 1, '0', 'i8') }}}
    } else if (result[1] !== -1) {
      {{{ makeSetValue('result[0]', 'result[1]', '0', 'i8') }}}
    }
    return result[0];
  },

  // ==========================================================================
  // sys/stat.h
  // ==========================================================================

  __stat_struct_layout: Runtime.generateStructInfo([
    'st_dev',
    'st_ino',
    'st_mode',
    'st_nlink',
    'st_uid',
    'st_gid',
    'st_rdev',
    'st_size',
    'st_atime',
    'st_spare1',
    'st_mtime',
    'st_spare2',
    'st_ctime',
    'st_spare3',
    'st_blksize',
    'st_blocks',
    'st_spare4'], '%struct.stat'),
  stat__deps: ['$FS', '__stat_struct_layout'],
  stat: function(path, buf, dontResolveLastLink) {
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/stat.html
    // int stat(const char *path, struct stat *buf);
    // NOTE: dontResolveLastLink is a shortcut for lstat(). It should never be
    //       used in client code.
    var obj = FS.findObject(Pointer_stringify(path), dontResolveLastLink);
    if (obj === null || !FS.forceLoadFile(obj)) return -1;

    var offsets = ___stat_struct_layout;

    // Constants.
    {{{ makeSetValue('buf', 'offsets.st_nlink', '1', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.st_uid', '0', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.st_gid', '0', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.st_blksize', '4096', 'i32') }}}

    // Variables.
    {{{ makeSetValue('buf', 'offsets.st_ino', 'obj.inodeNumber', 'i32') }}}
    var time = Math.floor(obj.timestamp / 1000);
    if (offsets.st_atime === undefined) {
      offsets.st_atime = offsets.st_atim.tv_sec;
      offsets.st_mtime = offsets.st_mtim.tv_sec;
      offsets.st_ctime = offsets.st_ctim.tv_sec;
      var nanosec = (obj.timestamp % 1000) * 1000;
      {{{ makeSetValue('buf', 'offsets.st_atim.tv_nsec', 'nanosec', 'i32') }}}
      {{{ makeSetValue('buf', 'offsets.st_mtim.tv_nsec', 'nanosec', 'i32') }}}
      {{{ makeSetValue('buf', 'offsets.st_ctim.tv_nsec', 'nanosec', 'i32') }}}
    }
    {{{ makeSetValue('buf', 'offsets.st_atime', 'time', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.st_mtime', 'time', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.st_ctime', 'time', 'i32') }}}
    var mode = 0;
    var size = 0;
    var blocks = 0;
    var dev = 0;
    var rdev = 0;
    if (obj.isDevice) {
      //  Device numbers reuse inode numbers.
      dev = rdev = obj.inodeNumber;
      size = blocks = 0;
      mode = 0x2000;  // S_IFCHR.
    } else {
      dev = 1;
      rdev = 0;
      // NOTE: In our implementation, st_blocks = Math.ceil(st_size/st_blksize),
      //       but this is not required by the standard.
      if (obj.isFolder) {
        size = 4096;
        blocks = 1;
        mode = 0x4000;  // S_IFDIR.
      } else {
        var data = obj.contents || obj.link;
        size = data.length;
        blocks = Math.ceil(data.length / 4096);
        mode = obj.link === undefined ? 0x8000 : 0xA000;  // S_IFREG, S_IFLNK.
      }
    }
    {{{ makeSetValue('buf', 'offsets.st_dev', 'dev', 'i32') }}};
    {{{ makeSetValue('buf', 'offsets.st_rdev', 'rdev', 'i32') }}};
    {{{ makeSetValue('buf', 'offsets.st_size', 'size', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.st_blocks', 'blocks', 'i32') }}}
    if (obj.read) mode |= 0x16D;  // S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH.
    if (obj.write) mode |= 0x92;  // S_IWUSR | S_IWGRP | S_IWOTH.
    {{{ makeSetValue('buf', 'offsets.st_mode', 'mode', 'i32') }}}

    return 0;
  },
  lstat__deps: ['stat'],
  lstat: function(path, buf) {
    // int lstat(const char *path, struct stat *buf);
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/lstat.html
    return _stat(path, buf, true);
  },
  fstat__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', 'stat'],
  fstat: function(fildes, buf) {
    // int fstat(int fildes, struct stat *buf);
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/fstat.html
    if (!FS.streams[fildes]) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    } else {
      var pathArray = intArrayFromString(FS.streams[fildes].path);
      return _stat(allocate(pathArray, 'i8', ALLOC_STACK), buf);
    }
  },
  mknod__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  mknod: function(path, mode, dev) {
    // int mknod(const char *path, mode_t mode, dev_t dev);
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/mknod.html
    if (dev !== 0 || !(mode & 0xC000)) {  // S_IFREG | S_IFDIR.
      // Can't create devices or pipes through mknod().
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      var properties = {contents: [], isFolder: Boolean(mode & 0x4000)};  // S_IFDIR.
      path = FS.analyzePath(Pointer_stringify(path));
      try {
        FS.createObject(path.parentObject, path.name, properties,
                        mode & 0x100, mode & 0x80);  // S_IRUSR, S_IWUSR.
        return 0;
      } catch (e) {
        return -1;
      }
    }
  },
  mkdir__deps: ['mknod'],
  mkdir: function(path, mode) {
    // int mkdir(const char *path, mode_t mode);
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/mkdir.html
    return _mknod(path, 0x4000 | (mode & 0x180), 0);  // S_IFDIR, S_IRUSR | S_IWUSR.
  },
  mkfifo__deps: ['__setErrNo', '$ERRNO_CODES'],
  mkfifo: function(path, mode) {
    // int mkfifo(const char *path, mode_t mode);
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/mkfifo.html
    // NOTE: We support running only a single process, and named pipes require
    //       blocking, which we can't provide. The error code is not very
    //       accurate, but it's the closest among those allowed in the standard
    //       and unlikely to result in retries.
    ___setErrNo(ERRNO_CODES.EROFS);
    return -1;
  },
  chmod__deps: ['$FS'],
  chmod: function(path, mode) {
    // int chmod(const char *path, mode_t mode);
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/chmod.html
    var obj = FS.findObject(Pointer_stringify(path));
    if (obj === null) return -1;
    obj.read = mode & 0x100;  // S_IRUSR.
    obj.write = mode & 0x80;  // S_IWUSR.
    obj.timestamp = Date.now();
    return 0;
  },
  fchmod__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', 'chmod'],
  fchmod: function(fildes, mode) {
    // int fchmod(int fildes, mode_t mode);
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/fchmod.html
    if (!FS.streams[fildes]) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    } else {
      var pathArray = intArrayFromString(FS.streams[fildes].path);
      return _chmod(allocate(pathArray, 'i8', ALLOC_STACK), mode);
    }
  },
  umask__deps: ['$FS'],
  umask: function(newMask) {
    // mode_t umask(mode_t cmask);
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/umask.html
    // NOTE: This value isn't actually used for anything.
    if (_umask.cmask === undefined) _umask.cmask = 0x1FF;  // S_IRWXU | S_IRWXG | S_IRWXO.
    var oldMask = _umask.cmask;
    _umask.cmask = newMask;
    return oldMask;
  },
  stat64: 'stat',
  fstat64: 'fstat',
  __01fstat64_: 'fstat',
  __01stat64_: 'stat',
  __01lstat64_: 'lstat',

  // TODO: Check if other aliases are needed.

  // ==========================================================================
  // sys/statvfs.h
  // ==========================================================================

  __statvfs_struct_layout: Runtime.generateStructInfo([
    'f_bsize',
    'f_frsize',
    'f_blocks',
    'f_bfree',
    'f_bavail',
    'f_files',
    'f_ffree',
    'f_favail',
    'f_fsid',
    'f_flag',
    'f_namemax'], '%struct.statvfs'),
  statvfs__deps: ['$FS', '__statvfs_struct_layout'],
  statvfs: function(path, buf) {
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/stat.html
    // int statvfs(const char *restrict path, struct statvfs *restrict buf);
    var offsets = ___statvfs_struct_layout;
    // NOTE: None of the constants here are true. We're just returning safe and
    //       sane values.
    {{{ makeSetValue('buf', 'offsets.f_bsize', '4096', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.f_frsize', '4096', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.f_blocks', '1000000', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.f_bfree', '500000', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.f_bavail', '500000', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.f_files', 'FS.nextInode', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.f_ffree', '1000000', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.f_favail', '1000000', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.f_fsid', '42', 'i32') }}}
    {{{ makeSetValue('buf', 'offsets.f_flag', '2', 'i32') }}}  // ST_NOSUID
    {{{ makeSetValue('buf', 'offsets.f_namemax', '255', 'i32') }}}
    return 0;
  },
  fstatvfs__deps: ['statvfs'],
  fstatvfs: function(fildes, buf) {
    // int fstatvfs(int fildes, struct statvfs *buf);
    // http://pubs.opengroup.org/onlinepubs/009604499/functions/statvfs.html
    return _statvfs(0, buf);
  },
  __01statvfs64_: 'statvfs',
  __01fstatvfs64_: 'fstatvfs',

  // ==========================================================================
  // fcntl.h
  // ==========================================================================

  __flock_struct_layout: Runtime.generateStructInfo([
    'l_type',
    'l_whence',
    'l_start',
    'l_len',
    'l_pid',
    'l_xxx'], '%struct.flock'),
  open__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', '__dirent_struct_layout'],
  open: function(path, oflag, varargs) {
    // int open(const char *path, int oflag, ...);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/open.html
    // NOTE: This implementation tries to mimic glibc rather that strictly
    // following the POSIX standard.

    var mode = {{{ makeGetValue('varargs', 0, 'i32') }}};

    // Simplify flags.
    var accessMode = oflag & {{{ cDefine('O_ACCMODE') }}};
    var isWrite = accessMode != {{{ cDefine('O_RDONLY') }}};
    var isRead = accessMode != {{{ cDefine('O_WRONLY') }}};
    var isCreate = Boolean(oflag & {{{ cDefine('O_CREAT') }}});
    var isExistCheck = Boolean(oflag & {{{ cDefine('O_EXCL') }}});
    var isTruncate = Boolean(oflag & {{{ cDefine('O_TRUNC') }}});
    var isAppend = Boolean(oflag & {{{ cDefine('O_APPEND') }}});

    // Verify path.
    var origPath = path;
    path = FS.analyzePath(Pointer_stringify(path));
    if (!path.parentExists) {
      ___setErrNo(path.error);
      return -1;
    }
    var target = path.object || null;
    var finalPath;

    // Verify the file exists, create if needed and allowed.
    if (target) {
      if (isCreate && isExistCheck) {
        ___setErrNo(ERRNO_CODES.EEXIST);
        return -1;
      }
      if ((isWrite || isCreate || isTruncate) && target.isFolder) {
        ___setErrNo(ERRNO_CODES.EISDIR);
        return -1;
      }
      if (isRead && !target.read || isWrite && !target.write) {
        ___setErrNo(ERRNO_CODES.EACCES);
        return -1;
      }
      if (isTruncate && !target.isDevice) {
        target.contents = [];
      } else {
        if (!FS.forceLoadFile(target)) {
          ___setErrNo(ERRNO_CODES.EIO);
          return -1;
        }
      }
      finalPath = path.path;
    } else {
      if (!isCreate) {
        ___setErrNo(ERRNO_CODES.ENOENT);
        return -1;
      }
      if (!path.parentObject.write) {
        ___setErrNo(ERRNO_CODES.EACCES);
        return -1;
      }
      target = FS.createDataFile(path.parentObject, path.name, [],
                                 mode & 0x100, mode & 0x80);  // S_IRUSR, S_IWUSR.
      finalPath = path.parentPath + '/' + path.name;
    }
    // Actually create an open stream.
    var id = FS.streams.length; // Keep dense
    if (target.isFolder) {
      var entryBuffer = 0;
      if (___dirent_struct_layout) {
        entryBuffer = _malloc(___dirent_struct_layout.__size__);
      }
      var contents = [];
      for (var key in target.contents) contents.push(key);
      FS.streams[id] = {
        path: finalPath,
        object: target,
        // An index into contents. Special values: -2 is ".", -1 is "..".
        position: -2,
        isRead: true,
        isWrite: false,
        isAppend: false,
        error: false,
        eof: false,
        ungotten: [],
        // Folder-specific properties:
        // Remember the contents at the time of opening in an array, so we can
        // seek between them relying on a single order.
        contents: contents,
        // Each stream has its own area for readdir() returns.
        currentEntry: entryBuffer
      };
    } else {
      FS.streams[id] = {
        path: finalPath,
        object: target,
        position: 0,
        isRead: isRead,
        isWrite: isWrite,
        isAppend: isAppend,
        error: false,
        eof: false,
        ungotten: []
      };
    }
#if ASSERTIONS
    FS.checkStreams();
#endif
    return id;
  },
  creat__deps: ['open'],
  creat: function(path, mode) {
    // int creat(const char *path, mode_t mode);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/creat.html
    return _open(path, {{{ cDefine('O_WRONLY') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_TRUNC') }}}, allocate([mode, 0, 0, 0], 'i32', ALLOC_STACK));
  },
  fcntl__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', '__flock_struct_layout'],
  fcntl: function(fildes, cmd, varargs, dup2) {
    // int fcntl(int fildes, int cmd, ...);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/fcntl.html
    if (!FS.streams[fildes]) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
    var stream = FS.streams[fildes];
    switch (cmd) {
      case {{{ cDefine('F_DUPFD') }}}:
        var arg = {{{ makeGetValue('varargs', 0, 'i32') }}};
        if (arg < 0) {
          ___setErrNo(ERRNO_CODES.EINVAL);
          return -1;
        }
        var newStream = {};
        for (var member in stream) {
          newStream[member] = stream[member];
        }
        arg = dup2 ? arg : Math.max(arg, FS.streams.length); // dup2 wants exactly arg; fcntl wants a free descriptor >= arg
        for (var i = FS.streams.length; i < arg; i++) {
          FS.streams[i] = null; // Keep dense
        }
        FS.streams[arg] = newStream;
#if ASSERTIONS
        FS.checkStreams();
#endif
        return arg;
      case {{{ cDefine('F_GETFD') }}}:
      case {{{ cDefine('F_SETFD') }}}:
        return 0;  // FD_CLOEXEC makes no sense for a single process.
      case {{{ cDefine('F_GETFL') }}}:
        var flags = 0;
        if (stream.isRead && stream.isWrite) flags = {{{ cDefine('O_RDWR') }}};
        else if (!stream.isRead && stream.isWrite) flags = {{{ cDefine('O_WRONLY') }}};
        else if (stream.isRead && !stream.isWrite) flags = {{{ cDefine('O_RDONLY') }}};
        if (stream.isAppend) flags |= {{{ cDefine('O_APPEND') }}};
        // Synchronization and blocking flags are irrelevant to us.
        return flags;
      case {{{ cDefine('F_SETFL') }}}:
        var arg = {{{ makeGetValue('varargs', 0, 'i32') }}};
        stream.isAppend = Boolean(arg | {{{ cDefine('O_APPEND') }}});
        // Synchronization and blocking flags are irrelevant to us.
        return 0;
      case {{{ cDefine('F_GETLK') }}}:
      case {{{ cDefine('F_GETLK64') }}}:
        var arg = {{{ makeGetValue('varargs', 0, 'i32') }}};
        var offset = ___flock_struct_layout.l_type;
        // We're always unlocked.
        {{{ makeSetValue('arg', 'offset', cDefine('F_UNLCK'), 'i16') }}}
        return 0;
      case {{{ cDefine('F_SETLK') }}}:
      case {{{ cDefine('F_SETLKW') }}}:
      case {{{ cDefine('F_SETLK64') }}}:
      case {{{ cDefine('F_SETLKW64') }}}:
        // Pretend that the locking is successful.
        return 0;
      case {{{ cDefine('F_SETOWN') }}}:
      case {{{ cDefine('F_GETOWN') }}}:
        // These are for sockets. We don't have them fully implemented yet.
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      default:
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
    }
    // Should never be reached. Only to silence strict warnings.
    return -1;
  },
  posix_fadvise: function(fd, offset, len, advice) {
    // int posix_fadvise(int fd, off_t offset, off_t len, int advice);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/posix_fadvise.html
    // Advise as much as you wish. We don't care.
    return 0;
  },
  posix_madvise: 'posix_fadvise',
  posix_fallocate__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  posix_fallocate: function(fd, offset, len) {
    // int posix_fallocate(int fd, off_t offset, off_t len);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/posix_fallocate.html
    if (!FS.streams[fd] || FS.streams[fd].link ||
        FS.streams[fd].isFolder || FS.streams[fd].isDevice) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
    var contents = FS.streams[fd].object.contents;
    var limit = offset + len;
    while (limit > contents.length) contents.push(0);
    return 0;
  },

  // ==========================================================================
  // poll.h
  // ==========================================================================

  __pollfd_struct_layout: Runtime.generateStructInfo(['fd', 'events', 'revents'], '%struct.pollfd'),
  poll__deps: ['$FS', '__pollfd_struct_layout'],
  poll: function(fds, nfds, timeout) {
    // int poll(struct pollfd fds[], nfds_t nfds, int timeout);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/poll.html
    // NOTE: This is pretty much a no-op mimicing glibc.
    var offsets = ___pollfd_struct_layout;
    var nonzero = 0;
    for (var i = 0; i < nfds; i++) {
      var pollfd = fds + ___pollfd_struct_layout.__size__ * i;
      var fd = {{{ makeGetValue('pollfd', 'offsets.fd', 'i32') }}};
      var events = {{{ makeGetValue('pollfd', 'offsets.events', 'i16') }}};
      var revents = 0;
      if (FS.streams[fd]) {
        var stream = FS.streams[fd];
        if (events & {{{ cDefine('POLLIN') }}}) revents |= {{{ cDefine('POLLIN') }}};
        if (events & {{{ cDefine('POLLOUT') }}}) revents |= {{{ cDefine('POLLOUT') }}};
      } else {
        if (events & {{{ cDefine('POLLNVAL') }}}) revents |= {{{ cDefine('POLLNVAL') }}};
      }
      if (revents) nonzero++;
      {{{ makeSetValue('pollfd', 'offsets.revents', 'revents', 'i16') }}}
    }
    return nonzero;
  },

  // ==========================================================================
  // unistd.h
  // ==========================================================================

  access__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  access: function(path, amode) {
    // int access(const char *path, int amode);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/access.html
    path = Pointer_stringify(path);
    var target = FS.findObject(path);
    if (target === null) return -1;
    if ((amode & 2 && !target.write) ||  // W_OK.
        ((amode & 1 || amode & 4) && !target.read)) {  // X_OK, R_OK.
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else {
      return 0;
    }
  },
  chdir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  chdir: function(path) {
    // int chdir(const char *path);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/chdir.html
    // NOTE: The path argument may be a string, to simplify fchdir().
    if (typeof path !== 'string') path = Pointer_stringify(path);
    path = FS.analyzePath(path);
    if (!path.exists) {
      ___setErrNo(path.error);
      return -1;
    } else if (!path.object.isFolder) {
      ___setErrNo(ERRNO_CODES.ENOTDIR);
      return -1;
    } else {
      FS.currentPath = path.path;
      return 0;
    }
  },
  chown__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  chown: function(path, owner, group, dontResolveLastLink) {
    // int chown(const char *path, uid_t owner, gid_t group);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/chown.html
    // We don't support multiple users, so changing ownership makes no sense.
    // NOTE: The path argument may be a string, to simplify fchown().
    // NOTE: dontResolveLastLink is a shortcut for lchown(). It should never be
    //       used in client code.
    if (typeof path !== 'string') path = Pointer_stringify(path);
    var target = FS.findObject(path, dontResolveLastLink);
    if (target === null) return -1;
    target.timestamp = Date.now();
    return 0;
  },
  chroot__deps: ['__setErrNo', '$ERRNO_CODES'],
  chroot: function(path) {
    // int chroot(const char *path);
    // http://pubs.opengroup.org/onlinepubs/7908799/xsh/chroot.html
    ___setErrNo(ERRNO_CODES.EACCES);
    return -1;
  },
  close__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  close: function(fildes) {
    // int close(int fildes);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/close.html
    if (FS.streams[fildes]) {
      if (FS.streams[fildes].currentEntry) {
        _free(FS.streams[fildes].currentEntry);
      }
      FS.streams[fildes] = null;
      return 0;
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
  },
  dup__deps: ['fcntl'],
  dup: function(fildes) {
    // int dup(int fildes);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/dup.html
    return _fcntl(fildes, 0, allocate([0, 0, 0, 0], 'i32', ALLOC_STACK));  // F_DUPFD.
  },
  dup2__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', 'fcntl', 'close'],
  dup2: function(fildes, fildes2) {
    // int dup2(int fildes, int fildes2);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/dup.html
    if (fildes2 < 0) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    } else if (fildes === fildes2 && FS.streams[fildes]) {
      return fildes;
    } else {
      _close(fildes2);
      return _fcntl(fildes, 0, allocate([fildes2, 0, 0, 0], 'i32', ALLOC_STACK), true);  // F_DUPFD.
    }
  },
  fchown__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', 'chown'],
  fchown: function(fildes, owner, group) {
    // int fchown(int fildes, uid_t owner, gid_t group);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fchown.html
    if (FS.streams[fildes]) {
      return _chown(FS.streams[fildes].path, owner, group);
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
  },
  fchdir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', 'chdir'],
  fchdir: function(fildes) {
    // int fchdir(int fildes);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fchdir.html
    if (FS.streams[fildes]) {
      return _chdir(FS.streams[fildes].path);
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
  },
  ctermid__deps: ['strcpy'],
  ctermid: function(s) {
    // char *ctermid(char *s);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/ctermid.html
    if (!_ctermid.ret) {
      var arr = intArrayFromString('/dev/tty');
      _ctermid.ret = allocate(arr, 'i8', ALLOC_NORMAL);
    }
    return s ? _strcpy(s, _ctermid.ret) : _ctermid.ret;
  },
  crypt: function(key, salt) {
    // char *(const char *, const char *);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/crypt.html
    // TODO: Implement (probably compile from C).
    ___setErrNo(ERRNO_CODES.ENOSYS);
    return 0;
  },
  encrypt: function(block, edflag) {
    // void encrypt(char block[64], int edflag);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/encrypt.html
    // TODO: Implement (probably compile from C).
    ___setErrNo(ERRNO_CODES.ENOSYS);
  },
  fpathconf__deps: ['__setErrNo', '$ERRNO_CODES'],
  fpathconf: function(fildes, name) {
    // long fpathconf(int fildes, int name);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/encrypt.html
    // NOTE: The first parameter is ignored, so pathconf == fpathconf.
    // The constants here aren't real values. Just mimicing glibc.
    switch (name) {
      case {{{ cDefine('_PC_LINK_MAX') }}}:
        return 32000;
      case {{{ cDefine('_PC_MAX_CANON') }}}:
      case {{{ cDefine('_PC_MAX_INPUT') }}}:
      case {{{ cDefine('_PC_NAME_MAX') }}}:
        return 255;
      case {{{ cDefine('_PC_PATH_MAX') }}}:
      case {{{ cDefine('_PC_PIPE_BUF') }}}:
      case {{{ cDefine('_PC_REC_MIN_XFER_SIZE') }}}:
      case {{{ cDefine('_PC_REC_XFER_ALIGN') }}}:
      case {{{ cDefine('_PC_ALLOC_SIZE_MIN') }}}:
        return 4096;
      case {{{ cDefine('_PC_CHOWN_RESTRICTED') }}}:
      case {{{ cDefine('_PC_NO_TRUNC') }}}:
      case {{{ cDefine('_PC_2_SYMLINKS') }}}:
        return 1;
      case {{{ cDefine('_PC_VDISABLE') }}}:
        return 0;
      case {{{ cDefine('_PC_SYNC_IO') }}}:
      case {{{ cDefine('_PC_ASYNC_IO') }}}:
      case {{{ cDefine('_PC_PRIO_IO') }}}:
      case {{{ cDefine('_PC_SOCK_MAXBUF') }}}:
      case {{{ cDefine('_PC_REC_INCR_XFER_SIZE') }}}:
      case {{{ cDefine('_PC_REC_MAX_XFER_SIZE') }}}:
      case {{{ cDefine('_PC_SYMLINK_MAX') }}}:
        return -1;
      case {{{ cDefine('_PC_FILESIZEBITS') }}}:
        return 64;
    }
    ___setErrNo(ERRNO_CODES.EINVAL);
    return -1;
  },
  pathconf: 'fpathconf',
  fsync__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  fsync: function(fildes) {
    // int fsync(int fildes);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fsync.html
    if (FS.streams[fildes]) {
      // We write directly to the file system, so there's nothing to do here.
      return 0;
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
  },
  fdatasync: 'fsync',
  truncate__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  truncate: function(path, length) {
    // int truncate(const char *path, off_t length);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/truncate.html
    // NOTE: The path argument may be a string, to simplify ftruncate().
    if (length < 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      if (typeof path !== 'string') path = Pointer_stringify(path);
      var target = FS.findObject(path);
      if (target === null) return -1;
      if (target.isFolder) {
        ___setErrNo(ERRNO_CODES.EISDIR);
        return -1;
      } else if (target.isDevice) {
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      } else if (!target.write) {
        ___setErrNo(ERRNO_CODES.EACCES);
        return -1;
      } else {
        var contents = target.contents;
        if (length < contents.length) contents.length = length;
        else while (length > contents.length) contents.push(0);
        target.timestamp = Date.now();
        return 0;
      }
    }
  },
  ftruncate__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', 'truncate'],
  ftruncate: function(fildes, length) {
    // int ftruncate(int fildes, off_t length);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/ftruncate.html
    if (FS.streams[fildes] && FS.streams[fildes].isWrite) {
      return _truncate(FS.streams[fildes].path, length);
    } else if (FS.streams[fildes]) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
  },
  getcwd__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  getcwd: function(buf, size) {
    // char *getcwd(char *buf, size_t size);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getcwd.html
    if (size == 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return 0;
    } else if (size < FS.currentPath.length + 1) {
      ___setErrNo(ERRNO_CODES.ERANGE);
      return 0;
    } else {
      for (var i = 0; i < FS.currentPath.length; i++) {
        {{{ makeSetValue('buf', 'i', 'FS.currentPath.charCodeAt(i)', 'i8') }}}
      }
      {{{ makeSetValue('buf', 'i', '0', 'i8') }}}
      return buf;
    }
  },
  getwd__deps: ['getcwd'],
  getwd: function(path_name) {
    // char *getwd(char *path_name);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getwd.html
    return _getcwd(path_name, 4096);  // PATH_MAX.
  },
  isatty__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  isatty: function(fildes) {
    // int isatty(int fildes);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/isatty.html
    if (!FS.streams[fildes]) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return 0;
    }
    if (FS.streams[fildes].isTerminal) return 1;
    ___setErrNo(ERRNO_CODES.ENOTTY);
    return 0;
  },
  lchown__deps: ['chown'],
  lchown: function(path, owner, group) {
    // int lchown(const char *path, uid_t owner, gid_t group);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/lchown.html
    return _chown(path, owner, group, true);
  },
  link__deps: ['__setErrNo', '$ERRNO_CODES'],
  link: function(path1, path2) {
    // int link(const char *path1, const char *path2);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/link.html
    // We don't support hard links.
    ___setErrNo(ERRNO_CODES.EMLINK);
    return -1;
  },
  lockf__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  lockf: function(fildes, func, size) {
    // int lockf(int fildes, int function, off_t size);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/lockf.html
    if (FS.streams[fildes]) {
      // Pretend whatever locking or unlocking operation succeeded. Locking does
      // not make much sense, since we have a single process/thread.
      return 0;
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
  },
  lseek__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  lseek: function(fildes, offset, whence) {
    // off_t lseek(int fildes, off_t offset, int whence);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/lseek.html
    if (FS.streams[fildes] && !FS.streams[fildes].object.isDevice) {
      var stream = FS.streams[fildes];
      var position = offset;
      if (whence === 1) {  // SEEK_CUR.
        position += stream.position;
      } else if (whence === 2) {  // SEEK_END.
        position += stream.object.contents.length;
      }
      if (position < 0) {
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
      } else {
        stream.ungotten = [];
        stream.position = position;
        return position;
      }
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
  },
  pipe__deps: ['__setErrNo', '$ERRNO_CODES'],
  pipe: function(fildes) {
    // int pipe(int fildes[2]);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/pipe.html
    // It is possible to implement this using two device streams, but pipes make
    // little sense in a single-threaded environment, so we do not support them.
    ___setErrNo(ERRNO_CODES.ENOSYS);
    return -1;
  },
  pread__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  pread: function(fildes, buf, nbyte, offset) {
    // ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/read.html
    var stream = FS.streams[fildes];
    if (!stream || stream.object.isDevice) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    } else if (!stream.isRead) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else if (stream.object.isFolder) {
      ___setErrNo(ERRNO_CODES.EISDIR);
      return -1;
    } else if (nbyte < 0 || offset < 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      var bytesRead = 0;
      while (stream.ungotten.length && nbyte > 0) {
        {{{ makeSetValue('buf++', '0', 'stream.ungotten.pop()', 'i8') }}}
        nbyte--;
        bytesRead++;
      }
      var contents = stream.object.contents;
      var size = Math.min(contents.length - offset, nbyte);
      if (contents.subarray || contents.slice) { // typed array or normal array
        for (var i = 0; i < size; i++) {
          {{{ makeSetValue('buf', 'i', 'contents[offset + i]', 'i8') }}}
        }
      } else {
        for (var i = 0; i < size; i++) { // LazyUint8Array from sync binary XHR
          {{{ makeSetValue('buf', 'i', 'contents.get(offset + i)', 'i8') }}}
        }
      }
      bytesRead += size;
      return bytesRead;
    }
  },
  read__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', 'pread'],
  read: function(fildes, buf, nbyte) {
    // ssize_t read(int fildes, void *buf, size_t nbyte);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/read.html
    var stream = FS.streams[fildes];
    if (!stream) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    } else if (!stream.isRead) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else if (nbyte < 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      var bytesRead;
      if (stream.object.isDevice) {
        if (stream.object.input) {
          bytesRead = 0;
          while (stream.ungotten.length && nbyte > 0) {
            {{{ makeSetValue('buf++', '0', 'stream.ungotten.pop()', 'i8') }}}
            nbyte--;
            bytesRead++;
          }
          for (var i = 0; i < nbyte; i++) {
            try {
              var result = stream.object.input();
            } catch (e) {
              ___setErrNo(ERRNO_CODES.EIO);
              return -1;
            }
            if (result === null || result === undefined) break;
            bytesRead++;
            {{{ makeSetValue('buf', 'i', 'result', 'i8') }}}
          }
          return bytesRead;
        } else {
          ___setErrNo(ERRNO_CODES.ENXIO);
          return -1;
        }
      } else {
        var ungotSize = stream.ungotten.length;
        bytesRead = _pread(fildes, buf, nbyte, stream.position);
        if (bytesRead != -1) {
          stream.position += (stream.ungotten.length - ungotSize) + bytesRead;
        }
        return bytesRead;
      }
    }
  },
  sync: function() {
    // void sync(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/sync.html
    // All our writing is already synchronized. This is a no-op.
  },
  rmdir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  rmdir: function(path) {
    // int rmdir(const char *path);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/rmdir.html
    path = FS.analyzePath(Pointer_stringify(path));
    if (!path.parentExists || !path.exists) {
      ___setErrNo(path.error);
      return -1;
    } else if (!path.object.write || path.isRoot) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else if (!path.object.isFolder) {
      ___setErrNo(ERRNO_CODES.ENOTDIR);
      return -1;
    } else {
      for (var i in path.object.contents) {
        ___setErrNo(ERRNO_CODES.ENOTEMPTY);
        return -1;
      }
      if (path.path == FS.currentPath) {
        ___setErrNo(ERRNO_CODES.EBUSY);
        return -1;
      } else {
        delete path.parentObject.contents[path.name];
        return 0;
      }
    }
  },
  unlink__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  unlink: function(path) {
    // int unlink(const char *path);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/unlink.html
    path = FS.analyzePath(Pointer_stringify(path));
    if (!path.parentExists || !path.exists) {
      ___setErrNo(path.error);
      return -1;
    } else if (path.object.isFolder) {
      ___setErrNo(ERRNO_CODES.EISDIR);
      return -1;
    } else if (!path.object.write) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else {
      delete path.parentObject.contents[path.name];
      return 0;
    }
  },
  ttyname__deps: ['ttyname_r'],
  ttyname: function(fildes) {
    // char *ttyname(int fildes);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/ttyname.html
    if (!_ttyname.ret) _ttyname.ret = _malloc(256);
    return _ttyname_r(fildes, _ttyname.ret, 256) ? 0 : _ttyname.ret;
  },
  ttyname_r__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  ttyname_r: function(fildes, name, namesize) {
    // int ttyname_r(int fildes, char *name, size_t namesize);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/ttyname.html
    var stream = FS.streams[fildes];
    if (!stream) {
      return ___setErrNo(ERRNO_CODES.EBADF);
    } else {
      var object = stream.object;
      if (!object.isDevice || !object.input || !object.output) {
        return ___setErrNo(ERRNO_CODES.ENOTTY);
      } else {
        var ret = stream.path;
        if (namesize < ret.length + 1) {
          return ___setErrNo(ERRNO_CODES.ERANGE);
        } else {
          for (var i = 0; i < ret.length; i++) {
            {{{ makeSetValue('name', 'i', 'ret.charCodeAt(i)', 'i8') }}}
          }
          {{{ makeSetValue('name', 'i', '0', 'i8') }}}
          return 0;
        }
      }
    }
  },
  symlink__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  symlink: function(path1, path2) {
    // int symlink(const char *path1, const char *path2);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/symlink.html
    var path = FS.analyzePath(Pointer_stringify(path2), true);
    if (!path.parentExists) {
      ___setErrNo(path.error);
      return -1;
    } else if (path.exists) {
      ___setErrNo(ERRNO_CODES.EEXIST);
      return -1;
    } else {
      FS.createLink(path.parentPath, path.name,
                    Pointer_stringify(path1), true, true);
      return 0;
    }
  },
  readlink__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  readlink: function(path, buf, bufsize) {
    // ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/readlink.html
    var target = FS.findObject(Pointer_stringify(path), true);
    if (target === null) return -1;
    if (target.link !== undefined) {
      var length = Math.min(bufsize - 1, target.link.length);
      for (var i = 0; i < length; i++) {
        {{{ makeSetValue('buf', 'i', 'target.link.charCodeAt(i)', 'i8') }}}
      }
      if (bufsize - 1 > length) {{{ makeSetValue('buf', 'i', '0', 'i8') }}}
      return i;
    } else {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
  },
  pwrite__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  pwrite: function(fildes, buf, nbyte, offset) {
    // ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/write.html
    var stream = FS.streams[fildes];
    if (!stream || stream.object.isDevice) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    } else if (!stream.isWrite) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else if (stream.object.isFolder) {
      ___setErrNo(ERRNO_CODES.EISDIR);
      return -1;
    } else if (nbyte < 0 || offset < 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      var contents = stream.object.contents;
      while (contents.length < offset) contents.push(0);
      for (var i = 0; i < nbyte; i++) {
        contents[offset + i] = {{{ makeGetValue('buf', 'i', 'i8', undefined, 1) }}};
      }
      stream.object.timestamp = Date.now();
      return i;
    }
  },
  write__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', 'pwrite'],
  write: function(fildes, buf, nbyte) {
    // ssize_t write(int fildes, const void *buf, size_t nbyte);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/write.html
    var stream = FS.streams[fildes];
    if (!stream) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    } else if (!stream.isWrite) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else if (nbyte < 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      if (stream.object.isDevice) {
        if (stream.object.output) {
          for (var i = 0; i < nbyte; i++) {
            try {
              stream.object.output({{{ makeGetValue('buf', 'i', 'i8') }}});
            } catch (e) {
              ___setErrNo(ERRNO_CODES.EIO);
              return -1;
            }
          }
          stream.object.timestamp = Date.now();
          return i;
        } else {
          ___setErrNo(ERRNO_CODES.ENXIO);
          return -1;
        }
      } else {
        var bytesWritten = _pwrite(fildes, buf, nbyte, stream.position);
        if (bytesWritten != -1) stream.position += bytesWritten;
        return bytesWritten;
      }
    }
  },
  alarm: function(seconds) {
    // unsigned alarm(unsigned seconds);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/alarm.html
    // We don't support signals, and there's no way to indicate failure, so just
    // fail silently.
    return 0;
  },
  ualarm: 'alarm',
  confstr__deps: ['__setErrNo', '$ERRNO_CODES', '$ENV'],
  confstr: function(name, buf, len) {
    // size_t confstr(int name, char *buf, size_t len);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/confstr.html
    var value;
    switch (name) {
      case {{{ cDefine('_CS_PATH') }}}:
        value = ENV['PATH'] || '/';
        break;
      case {{{ cDefine('_CS_POSIX_V6_WIDTH_RESTRICTED_ENVS') }}}:
        // Mimicing glibc.
        value = 'POSIX_V6_ILP32_OFF32\nPOSIX_V6_ILP32_OFFBIG';
        break;
      case {{{ cDefine('_CS_GNU_LIBC_VERSION') }}}:
        // This JS implementation was tested against this glibc version.
        value = 'glibc 2.14';
        break;
      case {{{ cDefine('_CS_GNU_LIBPTHREAD_VERSION') }}}:
        // We don't support pthreads.
        value = '';
        break;
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFF32_LIBS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFFBIG_LIBS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LP64_OFF64_CFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LP64_OFF64_LDFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LP64_OFF64_LIBS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_LPBIG_OFFBIG_LIBS') }}}:
        value = '';
        break;
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFF32_CFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFF32_LDFLAGS') }}}:
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS') }}}:
        value = '-m32';
        break;
      case {{{ cDefine('_CS_POSIX_V6_ILP32_OFFBIG_CFLAGS') }}}:
        value = '-m32 -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64';
        break;
      default:
        ___setErrNo(ERRNO_CODES.EINVAL);
        return 0;
    }
    if (len == 0 || buf == 0) {
      return value.length + 1;
    } else {
      var length = Math.min(len, value.length);
      for (var i = 0; i < length; i++) {
        {{{ makeSetValue('buf', 'i', 'value.charCodeAt(i)', 'i8') }}}
      }
      if (len > length) {{{ makeSetValue('buf', 'i++', '0', 'i8') }}}
      return i;
    }
  },
  execl__deps: ['__setErrNo', '$ERRNO_CODES'],
  execl: function(/* ... */) {
    // int execl(const char *path, const char *arg0, ... /*, (char *)0 */);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/exec.html
    // We don't support executing external code.
    ___setErrNo(ERRNO_CODES.ENOEXEC);
    return -1;
  },
  execle: 'execl',
  execlp: 'execl',
  execv: 'execl',
  execve: 'execl',
  execvp: 'execl',
  _exit: function(status) {
    // void _exit(int status);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/exit.html

#if CATCH_EXIT_CODE
    function ExitStatus() {
      this.name = "ExitStatus";
      this.message = "Program terminated with exit(" + status + ")";
      this.status = status;
    };
    ExitStatus.prototype = new Error();
    ExitStatus.prototype.constructor = ExitStatus;
#endif

    exitRuntime();
    ABORT = true;

#if CATCH_EXIT_CODE
    throw new ExitStatus();
#else
    throw 'exit(' + status + ') called, at ' + new Error().stack;
#endif
  },
  fork__deps: ['__setErrNo', '$ERRNO_CODES'],
  fork: function() {
    // pid_t fork(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fork.html
    // We don't support multiple processes.
    ___setErrNo(ERRNO_CODES.EAGAIN);
    return -1;
  },
  vfork: 'fork',
  getgid: function() {
    // gid_t getgid(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getgid.html
    // We have just one process/group/user, all with ID 0.
    return 0;
  },
  getegid: 'getgid',
  getuid: 'getgid',
  geteuid: 'getgid',
  getpgrp: 'getgid',
  getpid: 'getgid',
  getppid: 'getgid',
  getresuid: function(ruid, euid, suid) {
    // int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid);
    // http://linux.die.net/man/2/getresuid
    // We have just one process/group/user, all with ID 0.
    {{{ makeSetValue('ruid', '0', '0', 'i32') }}}
    {{{ makeSetValue('euid', '0', '0', 'i32') }}}
    {{{ makeSetValue('suid', '0', '0', 'i32') }}}
    return 0;
  },
  getresgid: 'getresuid',
  getgroups__deps: ['__setErrNo', '$ERRNO_CODES'],
  getgroups: function(gidsetsize, grouplist) {
    // int getgroups(int gidsetsize, gid_t grouplist[]);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getgroups.html
    if (gidsetsize < 1) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      {{{ makeSetValue('grouplist', '0', '0', 'i32') }}}
      return 1;
    }
  },
  // TODO: Implement initgroups, setgroups (grp.h).
  gethostid: function() {
    // long gethostid(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/gethostid.html
    return 42;
  },
  gethostname__deps: ['__setErrNo', '$ERRNO_CODES'],
  gethostname: function(name, namelen) {
    // int gethostname(char *name, size_t namelen);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/gethostname.html
    var host = 'emscripten';
    if (typeof window !== 'undefined' && window.location.host) {
      host = window.location.host;
    }
    var length = Math.min(namelen, host.length);
    for (var i = 0; i < length; i++) {
      {{{ makeSetValue('name', 'i', 'host.charCodeAt(i)', 'i8') }}}
    }
    if (namelen > length) {
      {{{ makeSetValue('name', 'i', '0', 'i8') }}}
      return 0;
    } else {
      ___setErrNo(ERRNO_CODES.ENAMETOOLONG);
      return -1;
    }
  },
  getlogin__deps: ['getlogin_r'],
  getlogin: function() {
    // char *getlogin(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getlogin.html
    if (!_getlogin.ret) _getlogin.ret = _malloc(8);
    return _getlogin_r(_getlogin.ret, 8) ? 0 : _getlogin.ret;
  },
  getlogin_r__deps: ['__setErrNo', '$ERRNO_CODES'],
  getlogin_r: function(name, namesize) {
    // int getlogin_r(char *name, size_t namesize);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getlogin.html
    var ret = 'root';
    if (namesize < ret.length + 1) {
      return ___setErrNo(ERRNO_CODES.ERANGE);
    } else {
      for (var i = 0; i < ret.length; i++) {
        {{{ makeSetValue('name', 'i', 'ret.charCodeAt(i)', 'i8') }}}
      }
      {{{ makeSetValue('name', 'i', '0', 'i8') }}}
      return 0;
    }
  },
  getpagesize: function() {
    // int getpagesize(void);
    return PAGE_SIZE;
  },
  getopt: function(argc, argv, optstring) {
    // int getopt(int argc, char * const argv[], const char *optstring);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getopt.html
    // TODO: Implement (probably compile from C).
    return -1;
  },
  getpgid: function(pid) {
    // pid_t getpgid(pid_t pid);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getpgid.html
    // There can only be one process, and its group ID is 0.
    return 0;
  },
  getsid: 'getpgid',
  nice__deps: ['__setErrNo', '$ERRNO_CODES'],
  nice: function(incr) {
    // int nice(int incr);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/nice.html
    // Niceness makes no sense in a single-process environment.
    ___setErrNo(ERRNO_CODES.EPERM);
    return 0;
  },
  pause__deps: ['__setErrNo', '$ERRNO_CODES'],
  pause: function() {
    // int pause(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/pause.html
    // We don't support signals, so we return immediately.
    ___setErrNo(ERRNO_CODES.EINTR);
    return -1;
  },
  setgid__deps: ['__setErrNo', '$ERRNO_CODES'],
  setgid: function(gid) {
    // int setgid(gid_t gid);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/setgid.html
    // We have just one process/group/user, so it makes no sense to set IDs.
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },
  setegid: 'setgid',
  setuid: 'setgid',
  seteuid: 'setgid',
  setsid: 'setgid',
  setpgrp: 'setgid',
  setpgid__deps: ['__setErrNo', '$ERRNO_CODES'],
  setpgid: function(pid, pgid) {
    // int setpgid(pid_t pid, pid_t pgid);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getpgid.html
    // We have just one process/group/user, so it makes no sense to set IDs.
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },
  setregid: 'setpgid',
  setreuid: 'setpgid',
  // NOTE: These do not match the signatures, but they all use the same stub.
  setresuid: 'setpgid',
  setresgid: 'setpgid',
  sleep__deps: ['usleep'],
  sleep: function(seconds) {
    // unsigned sleep(unsigned seconds);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/sleep.html
    return _usleep(seconds * 1e6);
  },
  usleep: function(useconds) {
    // int usleep(useconds_t useconds);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/usleep.html
    // We're single-threaded, so use a busy loop. Super-ugly.
    var msec = useconds / 1000;
    var start = Date.now();
    while (Date.now() - start < msec) {
      // Do nothing.
    }
    return 0;
  },
  swab: function(src, dest, nbytes) {
    // void swab(const void *restrict src, void *restrict dest, ssize_t nbytes);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/swab.html
    if (nbytes < 0) return;
    nbytes -= nbytes % 2;
    for (var i = 0; i < nbytes; i += 2) {
      var first = {{{ makeGetValue('src', 'i', 'i8') }}};
      var second = {{{ makeGetValue('src', 'i + 1', 'i8') }}};
      {{{ makeSetValue('dest', 'i', 'second', 'i8') }}}
      {{{ makeSetValue('dest', 'i + 1', 'first', 'i8') }}}
    }
  },
  tcgetpgrp: function(fildes) {
    // pid_t tcgetpgrp(int fildes);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/tcgetpgrp.html
    // Our only process always runs with group ID 0.
    return 0;
  },
  tcsetpgrp__deps: ['__setErrNo', '$ERRNO_CODES'],
  tcsetpgrp: function(fildes, pgid_id) {
    // int tcsetpgrp(int fildes, pid_t pgid_id);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/tcsetpgrp.html
    // We don't support multiple processes or groups with ID other than 0.
    ___setErrNo(ERRNO_CODES.EINVAL);
    return -1;
  },
  sysconf__deps: ['__setErrNo', '$ERRNO_CODES'],
  sysconf: function(name) {
    // long sysconf(int name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/sysconf.html
    switch(name) {
      case {{{ cDefine('_SC_PAGE_SIZE') }}}: return PAGE_SIZE;
      case {{{ cDefine('_SC_ADVISORY_INFO') }}}:
      case {{{ cDefine('_SC_BARRIERS') }}}:
      case {{{ cDefine('_SC_ASYNCHRONOUS_IO') }}}:
      case {{{ cDefine('_SC_CLOCK_SELECTION') }}}:
      case {{{ cDefine('_SC_CPUTIME') }}}:
      case {{{ cDefine('_SC_FSYNC') }}}:
      case {{{ cDefine('_SC_IPV6') }}}:
      case {{{ cDefine('_SC_MAPPED_FILES') }}}:
      case {{{ cDefine('_SC_MEMLOCK') }}}:
      case {{{ cDefine('_SC_MEMLOCK_RANGE') }}}:
      case {{{ cDefine('_SC_MEMORY_PROTECTION') }}}:
      case {{{ cDefine('_SC_MESSAGE_PASSING') }}}:
      case {{{ cDefine('_SC_MONOTONIC_CLOCK') }}}:
      case {{{ cDefine('_SC_PRIORITIZED_IO') }}}:
      case {{{ cDefine('_SC_PRIORITY_SCHEDULING') }}}:
      case {{{ cDefine('_SC_RAW_SOCKETS') }}}:
      case {{{ cDefine('_SC_READER_WRITER_LOCKS') }}}:
      case {{{ cDefine('_SC_REALTIME_SIGNALS') }}}:
      case {{{ cDefine('_SC_SEMAPHORES') }}}:
      case {{{ cDefine('_SC_SHARED_MEMORY_OBJECTS') }}}:
      case {{{ cDefine('_SC_SPAWN') }}}:
      case {{{ cDefine('_SC_SPIN_LOCKS') }}}:
      case {{{ cDefine('_SC_SYNCHRONIZED_IO') }}}:
      case {{{ cDefine('_SC_THREAD_ATTR_STACKADDR') }}}:
      case {{{ cDefine('_SC_THREAD_ATTR_STACKSIZE') }}}:
      case {{{ cDefine('_SC_THREAD_CPUTIME') }}}:
      case {{{ cDefine('_SC_THREAD_PRIO_INHERIT') }}}:
      case {{{ cDefine('_SC_THREAD_PRIO_PROTECT') }}}:
      case {{{ cDefine('_SC_THREAD_PRIORITY_SCHEDULING') }}}:
      case {{{ cDefine('_SC_THREAD_PROCESS_SHARED') }}}:
      case {{{ cDefine('_SC_THREAD_SAFE_FUNCTIONS') }}}:
      case {{{ cDefine('_SC_THREADS') }}}:
      case {{{ cDefine('_SC_TIMEOUTS') }}}:
      case {{{ cDefine('_SC_TIMERS') }}}:
      case {{{ cDefine('_SC_VERSION') }}}:
      case {{{ cDefine('_SC_2_C_BIND') }}}:
      case {{{ cDefine('_SC_2_C_DEV') }}}:
      case {{{ cDefine('_SC_2_CHAR_TERM') }}}:
      case {{{ cDefine('_SC_2_LOCALEDEF') }}}:
      case {{{ cDefine('_SC_2_SW_DEV') }}}:
      case {{{ cDefine('_SC_2_VERSION') }}}:
        return 200809;
      case {{{ cDefine('_SC_MQ_OPEN_MAX') }}}:
      case {{{ cDefine('_SC_XOPEN_STREAMS') }}}:
      case {{{ cDefine('_SC_XBS5_LP64_OFF64') }}}:
      case {{{ cDefine('_SC_XBS5_LPBIG_OFFBIG') }}}:
      case {{{ cDefine('_SC_AIO_LISTIO_MAX') }}}:
      case {{{ cDefine('_SC_AIO_MAX') }}}:
      case {{{ cDefine('_SC_SPORADIC_SERVER') }}}:
      case {{{ cDefine('_SC_THREAD_SPORADIC_SERVER') }}}:
      case {{{ cDefine('_SC_TRACE') }}}:
      case {{{ cDefine('_SC_TRACE_EVENT_FILTER') }}}:
      case {{{ cDefine('_SC_TRACE_EVENT_NAME_MAX') }}}:
      case {{{ cDefine('_SC_TRACE_INHERIT') }}}:
      case {{{ cDefine('_SC_TRACE_LOG') }}}:
      case {{{ cDefine('_SC_TRACE_NAME_MAX') }}}:
      case {{{ cDefine('_SC_TRACE_SYS_MAX') }}}:
      case {{{ cDefine('_SC_TRACE_USER_EVENT_MAX') }}}:
      case {{{ cDefine('_SC_TYPED_MEMORY_OBJECTS') }}}:
      case {{{ cDefine('_SC_V6_LP64_OFF64') }}}:
      case {{{ cDefine('_SC_V6_LPBIG_OFFBIG') }}}:
      case {{{ cDefine('_SC_2_FORT_DEV') }}}:
      case {{{ cDefine('_SC_2_FORT_RUN') }}}:
      case {{{ cDefine('_SC_2_PBS') }}}:
      case {{{ cDefine('_SC_2_PBS_ACCOUNTING') }}}:
      case {{{ cDefine('_SC_2_PBS_CHECKPOINT') }}}:
      case {{{ cDefine('_SC_2_PBS_LOCATE') }}}:
      case {{{ cDefine('_SC_2_PBS_MESSAGE') }}}:
      case {{{ cDefine('_SC_2_PBS_TRACK') }}}:
      case {{{ cDefine('_SC_2_UPE') }}}:
      case {{{ cDefine('_SC_THREAD_THREADS_MAX') }}}:
      case {{{ cDefine('_SC_SEM_NSEMS_MAX') }}}:
      case {{{ cDefine('_SC_SYMLOOP_MAX') }}}:
      case {{{ cDefine('_SC_TIMER_MAX') }}}:
        return -1;
      case {{{ cDefine('_SC_V6_ILP32_OFF32') }}}:
      case {{{ cDefine('_SC_V6_ILP32_OFFBIG') }}}:
      case {{{ cDefine('_SC_JOB_CONTROL') }}}:
      case {{{ cDefine('_SC_REGEXP') }}}:
      case {{{ cDefine('_SC_SAVED_IDS') }}}:
      case {{{ cDefine('_SC_SHELL') }}}:
      case {{{ cDefine('_SC_XBS5_ILP32_OFF32') }}}:
      case {{{ cDefine('_SC_XBS5_ILP32_OFFBIG') }}}:
      case {{{ cDefine('_SC_XOPEN_CRYPT') }}}:
      case {{{ cDefine('_SC_XOPEN_ENH_I18N') }}}:
      case {{{ cDefine('_SC_XOPEN_LEGACY') }}}:
      case {{{ cDefine('_SC_XOPEN_REALTIME') }}}:
      case {{{ cDefine('_SC_XOPEN_REALTIME_THREADS') }}}:
      case {{{ cDefine('_SC_XOPEN_SHM') }}}:
      case {{{ cDefine('_SC_XOPEN_UNIX') }}}:
        return 1;
      case {{{ cDefine('_SC_THREAD_KEYS_MAX') }}}:
      case {{{ cDefine('_SC_IOV_MAX') }}}:
      case {{{ cDefine('_SC_GETGR_R_SIZE_MAX') }}}:
      case {{{ cDefine('_SC_GETPW_R_SIZE_MAX') }}}:
      case {{{ cDefine('_SC_OPEN_MAX') }}}:
        return 1024;
      case {{{ cDefine('_SC_RTSIG_MAX') }}}:
      case {{{ cDefine('_SC_EXPR_NEST_MAX') }}}:
      case {{{ cDefine('_SC_TTY_NAME_MAX') }}}:
        return 32;
      case {{{ cDefine('_SC_ATEXIT_MAX') }}}:
      case {{{ cDefine('_SC_DELAYTIMER_MAX') }}}:
      case {{{ cDefine('_SC_SEM_VALUE_MAX') }}}:
        return 2147483647;
      case {{{ cDefine('_SC_SIGQUEUE_MAX') }}}:
      case {{{ cDefine('_SC_CHILD_MAX') }}}:
        return 47839;
      case {{{ cDefine('_SC_BC_SCALE_MAX') }}}:
      case {{{ cDefine('_SC_BC_BASE_MAX') }}}:
        return 99;
      case {{{ cDefine('_SC_LINE_MAX') }}}:
      case {{{ cDefine('_SC_BC_DIM_MAX') }}}:
        return 2048;
      case {{{ cDefine('_SC_ARG_MAX') }}}: return 2097152;
      case {{{ cDefine('_SC_NGROUPS_MAX') }}}: return 65536;
      case {{{ cDefine('_SC_MQ_PRIO_MAX') }}}: return 32768;
      case {{{ cDefine('_SC_RE_DUP_MAX') }}}: return 32767;
      case {{{ cDefine('_SC_THREAD_STACK_MIN') }}}: return 16384;
      case {{{ cDefine('_SC_BC_STRING_MAX') }}}: return 1000;
      case {{{ cDefine('_SC_XOPEN_VERSION') }}}: return 700;
      case {{{ cDefine('_SC_LOGIN_NAME_MAX') }}}: return 256;
      case {{{ cDefine('_SC_COLL_WEIGHTS_MAX') }}}: return 255;
      case {{{ cDefine('_SC_CLK_TCK') }}}: return 100;
      case {{{ cDefine('_SC_HOST_NAME_MAX') }}}: return 64;
      case {{{ cDefine('_SC_AIO_PRIO_DELTA_MAX') }}}: return 20;
      case {{{ cDefine('_SC_STREAM_MAX') }}}: return 16;
      case {{{ cDefine('_SC_TZNAME_MAX') }}}: return 6;
      case {{{ cDefine('_SC_THREAD_DESTRUCTOR_ITERATIONS') }}}: return 4;
    }
    ___setErrNo(ERRNO_CODES.EINVAL);
    return -1;
  },
  sbrk: function(bytes) {
    // Implement a Linux-like 'memory area' for our 'process'.
    // Changes the size of the memory area by |bytes|; returns the
    // address of the previous top ('break') of the memory area

    // We need to make sure no one else allocates unfreeable memory!
    // We must control this entirely. So we don't even need to do
    // unfreeable allocations - the HEAP is ours, from STATICTOP up.
    // TODO: We could in theory slice off the top of the HEAP when
    //       sbrk gets a negative increment in |bytes|...
    var self = _sbrk;
    if (!self.called) {
      STATICTOP = alignMemoryPage(STATICTOP); // make sure we start out aligned
      self.called = true;
#if GC_SUPPORT
      _sbrk.DYNAMIC_START = STATICTOP;
#endif
    }
    var ret = STATICTOP;
    if (bytes != 0) Runtime.staticAlloc(bytes);
    return ret;  // Previous break location.
  },
  open64: 'open',
  lseek64: 'lseek',
  ftruncate64: 'ftruncate',
  __01open64_: 'open',
  __01lseek64_: 'lseek',
  __01truncate64_: 'truncate',
  __01ftruncate64_: 'ftruncate',
  // TODO: Check if any other aliases are needed.

  // ==========================================================================
  // stdio.h
  // ==========================================================================

  _isFloat: function(text) {
    return !!(/^[+-]?[0-9]*\.?[0-9]+([eE][+-]?[0-9]+)?$/.exec(text));
  },

  // TODO: Document.
  _scanString__deps: ['_isFloat'],
  _scanString: function(format, get, unget, varargs) {
    if (!__scanString.whiteSpace) {
      __scanString.whiteSpace = {};
      __scanString.whiteSpace[' '.charCodeAt(0)] = 1;
      __scanString.whiteSpace['\t'.charCodeAt(0)] = 1;
      __scanString.whiteSpace['\n'.charCodeAt(0)] = 1;
      __scanString.whiteSpace[' '] = 1;
      __scanString.whiteSpace['\t'] = 1;
      __scanString.whiteSpace['\n'] = 1;
    }
    // Supports %x, %4x, %d.%d, %lld, %s, %f, %lf.
    // TODO: Support all format specifiers.
    format = Pointer_stringify(format);
    var soFar = 0;
    if (format.indexOf('%n') >= 0) {
      // need to track soFar
      var _get = get;
      get = function() {
        soFar++;
        return _get();
      }
      var _unget = unget;
      unget = function() {
        soFar--;
        return _unget();
      }
    }
    var formatIndex = 0;
    var argsi = 0;
    var fields = 0;
    var argIndex = 0;
    var next;

    mainLoop:
    for (var formatIndex = 0; formatIndex < format.length;) {
      if (format[formatIndex] === '%' && format[formatIndex+1] == 'n') {
        var argPtr = {{{ makeGetValue('varargs', 'argIndex', 'void*') }}};
        argIndex += Runtime.getNativeFieldSize('void*');
        {{{ makeSetValue('argPtr', 0, 'soFar', 'i32') }}};
        formatIndex += 2;
        continue;
      }

      // remove whitespace
      while (1) {
        next = get();
        if (next == 0) return fields;
        if (!(next in __scanString.whiteSpace)) break;
      }
      unget();

      if (format[formatIndex] === '%') {
        formatIndex++;
        var maxSpecifierStart = formatIndex;
        while (format[formatIndex].charCodeAt(0) >= '0'.charCodeAt(0) &&
               format[formatIndex].charCodeAt(0) <= '9'.charCodeAt(0)) {
          formatIndex++;
        }
        var max_;
        if (formatIndex != maxSpecifierStart) {
          max_ = parseInt(format.slice(maxSpecifierStart, formatIndex), 10);
        }
        var long_ = false;
        var half = false;
        var longLong = false;
        if (format[formatIndex] == 'l') {
          long_ = true;
          formatIndex++;
          if(format[formatIndex] == 'l') {
            longLong = true;
            formatIndex++;
          }
        } else if (format[formatIndex] == 'h') {
          half = true;
          formatIndex++;
        }
        var type = format[formatIndex];
        formatIndex++;
        var curr = 0;
        var buffer = [];
        // Read characters according to the format. floats are trickier, they may be in an unfloat state in the middle, then be a valid float later
        if (type == 'f') {
          var last = 0;
          next = get();
          while (next > 0) {
            buffer.push(String.fromCharCode(next));
            if (__isFloat(buffer.join(''))) {
              last = buffer.length;
            }
            next = get();
          }
          for (var i = 0; i < buffer.length - last + 1; i++) {
            unget();
          }
          buffer.length = last;
        } else {
          next = get();
          var first = true;
          while ((curr < max_ || isNaN(max_)) && next > 0) {
            if (!(next in __scanString.whiteSpace) && // stop on whitespace
                (type == 's' ||
                 ((type === 'd' || type == 'u' || type == 'i') && ((next >= '0'.charCodeAt(0) && next <= '9'.charCodeAt(0)) ||
                                                                   (first && next == '-'.charCodeAt(0)))) ||
                 (type === 'x' && (next >= '0'.charCodeAt(0) && next <= '9'.charCodeAt(0) ||
                                   next >= 'a'.charCodeAt(0) && next <= 'f'.charCodeAt(0) ||
                                   next >= 'A'.charCodeAt(0) && next <= 'F'.charCodeAt(0)))) &&
                (formatIndex >= format.length || next !== format[formatIndex].charCodeAt(0))) { // Stop when we read something that is coming up
              buffer.push(String.fromCharCode(next));
              next = get();
              curr++;
              first = false;
            } else {
              break;
            }
          }
          unget();
        }
        if (buffer.length === 0) return 0;  // Failure.
        var text = buffer.join('');
        var argPtr = {{{ makeGetValue('varargs', 'argIndex', 'void*') }}};
        argIndex += Runtime.getNativeFieldSize('void*');
        switch (type) {
          case 'd': case 'u': case 'i':
            if (half) {
              {{{ makeSetValue('argPtr', 0, 'parseInt(text, 10)', 'i16') }}};
            } else if(longLong) {
              {{{ makeSetValue('argPtr', 0, 'parseInt(text, 10)', 'i64') }}};
            } else {
              {{{ makeSetValue('argPtr', 0, 'parseInt(text, 10)', 'i32') }}};
            }
            break;
          case 'x':
            {{{ makeSetValue('argPtr', 0, 'parseInt(text, 16)', 'i32') }}}
            break;
          case 'f':
            if (long_) {
              {{{ makeSetValue('argPtr', 0, 'parseFloat(text)', 'double') }}}
            } else {
              {{{ makeSetValue('argPtr', 0, 'parseFloat(text)', 'float') }}}
            }
            break;
          case 's':
            var array = intArrayFromString(text);
            for (var j = 0; j < array.length; j++) {
              {{{ makeSetValue('argPtr', 'j', 'array[j]', 'i8') }}}
            }
            break;
        }
        fields++;
      } else if (format[formatIndex] in __scanString.whiteSpace) {
        next = get();
        while (next in __scanString.whiteSpace) {
          if (next <= 0) break mainLoop;  // End of input.
          next = get();
        }
        unget(next);
        formatIndex++;
      } else {
        // Not a specifier.
        next = get();
        if (format[formatIndex].charCodeAt(0) !== next) {
          unget(next);
          break mainLoop;
        }
        formatIndex++;
      }
    }
    return fields;
  },
  // Performs prtinf-style formatting.
  //   format: A pointer to the format string.
  //   varargs: A pointer to the start of the arguments list.
  // Returns the resulting string string as a character array.
  _formatString: function(format, varargs) {
    var textIndex = format;
    var argIndex = 0;
    function getNextArg(type) {
      // NOTE: Explicitly ignoring type safety. Otherwise this fails:
      //       int x = 4; printf("%c\n", (char)x);
      var ret;
      if (type === 'double') {
        ret = {{{ makeGetValue('varargs', 'argIndex', 'double', undefined, undefined, true) }}};
#if USE_TYPED_ARRAYS == 2
      } else if (type == 'i64') {
        ret = [{{{ makeGetValue('varargs', 'argIndex', 'i32', undefined, undefined, true) }}},
               {{{ makeGetValue('varargs', 'argIndex+4', 'i32', undefined, undefined, true) }}}];
#else
      } else if (type == 'i64') {
        ret = {{{ makeGetValue('varargs', 'argIndex', 'i64', undefined, undefined, true) }}};
#endif
      } else {
        type = 'i32'; // varargs are always i32, i64, or double
        ret = {{{ makeGetValue('varargs', 'argIndex', 'i32', undefined, undefined, true) }}};
      }
      argIndex += Runtime.getNativeFieldSize(type);
      return ret;
    }

    var ret = [];
    var curr, next, currArg;
    while(1) {
      var startTextIndex = textIndex;
      curr = {{{ makeGetValue(0, 'textIndex', 'i8') }}};
      if (curr === 0) break;
      next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
      if (curr == '%'.charCodeAt(0)) {
        // Handle flags.
        var flagAlwaysSigned = false;
        var flagLeftAlign = false;
        var flagAlternative = false;
        var flagZeroPad = false;
        flagsLoop: while (1) {
          switch (next) {
            case '+'.charCodeAt(0):
              flagAlwaysSigned = true;
              break;
            case '-'.charCodeAt(0):
              flagLeftAlign = true;
              break;
            case '#'.charCodeAt(0):
              flagAlternative = true;
              break;
            case '0'.charCodeAt(0):
              if (flagZeroPad) {
                break flagsLoop;
              } else {
                flagZeroPad = true;
                break;
              }
            default:
              break flagsLoop;
          }
          textIndex++;
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
        }

        // Handle width.
        var width = 0;
        if (next == '*'.charCodeAt(0)) {
          width = getNextArg('i32');
          textIndex++;
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
        } else {
          while (next >= '0'.charCodeAt(0) && next <= '9'.charCodeAt(0)) {
            width = width * 10 + (next - '0'.charCodeAt(0));
            textIndex++;
            next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
          }
        }

        // Handle precision.
        var precisionSet = false;
        if (next == '.'.charCodeAt(0)) {
          var precision = 0;
          precisionSet = true;
          textIndex++;
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
          if (next == '*'.charCodeAt(0)) {
            precision = getNextArg('i32');
            textIndex++;
          } else {
            while(1) {
              var precisionChr = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
              if (precisionChr < '0'.charCodeAt(0) ||
                  precisionChr > '9'.charCodeAt(0)) break;
              precision = precision * 10 + (precisionChr - '0'.charCodeAt(0));
              textIndex++;
            }
          }
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
        } else {
          var precision = 6; // Standard default.
        }

        // Handle integer sizes. WARNING: These assume a 32-bit architecture!
        var argSize;
        switch (String.fromCharCode(next)) {
          case 'h':
            var nextNext = {{{ makeGetValue(0, 'textIndex+2', 'i8') }}};
            if (nextNext == 'h'.charCodeAt(0)) {
              textIndex++;
              argSize = 1; // char (actually i32 in varargs)
            } else {
              argSize = 2; // short (actually i32 in varargs)
            }
            break;
          case 'l':
            var nextNext = {{{ makeGetValue(0, 'textIndex+2', 'i8') }}};
            if (nextNext == 'l'.charCodeAt(0)) {
              textIndex++;
              argSize = 8; // long long
            } else {
              argSize = 4; // long
            }
            break;
          case 'L': // long long
          case 'q': // int64_t
          case 'j': // intmax_t
            argSize = 8;
            break;
          case 'z': // size_t
          case 't': // ptrdiff_t
          case 'I': // signed ptrdiff_t or unsigned size_t
            argSize = 4;
            break;
          default:
            argSize = null;
        }
        if (argSize) textIndex++;
        next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};

        // Handle type specifier.
        if (['d', 'i', 'u', 'o', 'x', 'X', 'p'].indexOf(String.fromCharCode(next)) != -1) {
          // Integer.
          var signed = next == 'd'.charCodeAt(0) || next == 'i'.charCodeAt(0);
          argSize = argSize || 4;
          var currArg = getNextArg('i' + (argSize * 8));
#if PRECISE_I64_MATH
          var origArg = currArg;
#endif
          var argText;
#if USE_TYPED_ARRAYS == 2
          // Flatten i64-1 [low, high] into a (slightly rounded) double
          if (argSize == 8) {
            currArg = Runtime.makeBigInt(currArg[0], currArg[1], next == 'u'.charCodeAt(0));
          }
#endif
          // Truncate to requested size.
          if (argSize <= 4) {
            var limit = Math.pow(256, argSize) - 1;
            currArg = (signed ? reSign : unSign)(currArg & limit, argSize * 8);
          }
          // Format the number.
          var currAbsArg = Math.abs(currArg);
          var prefix = '';
          if (next == 'd'.charCodeAt(0) || next == 'i'.charCodeAt(0)) {
#if PRECISE_I64_MATH
            if (argSize == 8 && i64Math) argText = i64Math.stringify(origArg[0], origArg[1], null); else
#endif
            argText = reSign(currArg, 8 * argSize, 1).toString(10);
          } else if (next == 'u'.charCodeAt(0)) {
#if PRECISE_I64_MATH
            if (argSize == 8 && i64Math) argText = i64Math.stringify(origArg[0], origArg[1], true); else
#endif
            argText = unSign(currArg, 8 * argSize, 1).toString(10);
            currArg = Math.abs(currArg);
          } else if (next == 'o'.charCodeAt(0)) {
            argText = (flagAlternative ? '0' : '') + currAbsArg.toString(8);
          } else if (next == 'x'.charCodeAt(0) || next == 'X'.charCodeAt(0)) {
            prefix = flagAlternative ? '0x' : '';
#if PRECISE_I64_MATH
            if (argSize == 8 && i64Math) argText = (origArg[1]>>>0).toString(16) + (origArg[0]>>>0).toString(16); else
#endif
            if (currArg < 0) {
              // Represent negative numbers in hex as 2's complement.
              currArg = -currArg;
              argText = (currAbsArg - 1).toString(16);
              var buffer = [];
              for (var i = 0; i < argText.length; i++) {
                buffer.push((0xF - parseInt(argText[i], 16)).toString(16));
              }
              argText = buffer.join('');
              while (argText.length < argSize * 2) argText = 'f' + argText;
            } else {
              argText = currAbsArg.toString(16);
            }
            if (next == 'X'.charCodeAt(0)) {
              prefix = prefix.toUpperCase();
              argText = argText.toUpperCase();
            }
          } else if (next == 'p'.charCodeAt(0)) {
            if (currAbsArg === 0) {
              argText = '(nil)';
            } else {
              prefix = '0x';
              argText = currAbsArg.toString(16);
            }
          }
          if (precisionSet) {
            while (argText.length < precision) {
              argText = '0' + argText;
            }
          }

          // Add sign if needed
          if (flagAlwaysSigned) {
            if (currArg < 0) {
              prefix = '-' + prefix;
            } else {
              prefix = '+' + prefix;
            }
          }

          // Add padding.
          while (prefix.length + argText.length < width) {
            if (flagLeftAlign) {
              argText += ' ';
            } else {
              if (flagZeroPad) {
                argText = '0' + argText;
              } else {
                prefix = ' ' + prefix;
              }
            }
          }

          // Insert the result into the buffer.
          argText = prefix + argText;
          argText.split('').forEach(function(chr) {
            ret.push(chr.charCodeAt(0));
          });
        } else if (['f', 'F', 'e', 'E', 'g', 'G'].indexOf(String.fromCharCode(next)) != -1) {
          // Float.
          var currArg = getNextArg('double');
          var argText;

          if (isNaN(currArg)) {
            argText = 'nan';
            flagZeroPad = false;
          } else if (!isFinite(currArg)) {
            argText = (currArg < 0 ? '-' : '') + 'inf';
            flagZeroPad = false;
          } else {
            var isGeneral = false;
            var effectivePrecision = Math.min(precision, 20);

            // Convert g/G to f/F or e/E, as per:
            // http://pubs.opengroup.org/onlinepubs/9699919799/functions/printf.html
            if (next == 'g'.charCodeAt(0) || next == 'G'.charCodeAt(0)) {
              isGeneral = true;
              precision = precision || 1;
              var exponent = parseInt(currArg.toExponential(effectivePrecision).split('e')[1], 10);
              if (precision > exponent && exponent >= -4) {
                next = ((next == 'g'.charCodeAt(0)) ? 'f' : 'F').charCodeAt(0);
                precision -= exponent + 1;
              } else {
                next = ((next == 'g'.charCodeAt(0)) ? 'e' : 'E').charCodeAt(0);
                precision--;
              }
              effectivePrecision = Math.min(precision, 20);
            }

            if (next == 'e'.charCodeAt(0) || next == 'E'.charCodeAt(0)) {
              argText = currArg.toExponential(effectivePrecision);
              // Make sure the exponent has at least 2 digits.
              if (/[eE][-+]\d$/.test(argText)) {
                argText = argText.slice(0, -1) + '0' + argText.slice(-1);
              }
            } else if (next == 'f'.charCodeAt(0) || next == 'F'.charCodeAt(0)) {
              argText = currArg.toFixed(effectivePrecision);
            }

            var parts = argText.split('e');
            if (isGeneral && !flagAlternative) {
              // Discard trailing zeros and periods.
              while (parts[0].length > 1 && parts[0].indexOf('.') != -1 &&
                     (parts[0].slice(-1) == '0' || parts[0].slice(-1) == '.')) {
                parts[0] = parts[0].slice(0, -1);
              }
            } else {
              // Make sure we have a period in alternative mode.
              if (flagAlternative && argText.indexOf('.') == -1) parts[0] += '.';
              // Zero pad until required precision.
              while (precision > effectivePrecision++) parts[0] += '0';
            }
            argText = parts[0] + (parts.length > 1 ? 'e' + parts[1] : '');

            // Capitalize 'E' if needed.
            if (next == 'E'.charCodeAt(0)) argText = argText.toUpperCase();

            // Add sign.
            if (flagAlwaysSigned && currArg >= 0) {
              argText = '+' + argText;
            }
          }

          // Add padding.
          while (argText.length < width) {
            if (flagLeftAlign) {
              argText += ' ';
            } else {
              if (flagZeroPad && (argText[0] == '-' || argText[0] == '+')) {
                argText = argText[0] + '0' + argText.slice(1);
              } else {
                argText = (flagZeroPad ? '0' : ' ') + argText;
              }
            }
          }

          // Adjust case.
          if (next < 'a'.charCodeAt(0)) argText = argText.toUpperCase();

          // Insert the result into the buffer.
          argText.split('').forEach(function(chr) {
            ret.push(chr.charCodeAt(0));
          });
        } else if (next == 's'.charCodeAt(0)) {
          // String.
          var arg = getNextArg('i8*') || nullString;
          var argLength = String_len(arg);
          if (precisionSet) argLength = Math.min(argLength, precision);
          if (!flagLeftAlign) {
            while (argLength < width--) {
              ret.push(' '.charCodeAt(0));
            }
          }
          for (var i = 0; i < argLength; i++) {
            ret.push({{{ makeGetValue('arg++', 0, 'i8', null, true) }}});
          }
          if (flagLeftAlign) {
            while (argLength < width--) {
              ret.push(' '.charCodeAt(0));
            }
          }
        } else if (next == 'c'.charCodeAt(0)) {
          // Character.
          if (flagLeftAlign) ret.push(getNextArg('i8'));
          while (--width > 0) {
            ret.push(' '.charCodeAt(0));
          }
          if (!flagLeftAlign) ret.push(getNextArg('i8'));
        } else if (next == 'n'.charCodeAt(0)) {
          // Write the length written so far to the next parameter.
          var ptr = getNextArg('i32*');
          {{{ makeSetValue('ptr', '0', 'ret.length', 'i32') }}}
        } else if (next == '%'.charCodeAt(0)) {
          // Literal percent sign.
          ret.push(curr);
        } else {
          // Unknown specifiers remain untouched.
          for (var i = startTextIndex; i < textIndex + 2; i++) {
            ret.push({{{ makeGetValue(0, 'i', 'i8') }}});
          }
        }
        textIndex += 2;
        // TODO: Support a/A (hex float) and m (last error) specifiers.
        // TODO: Support %1${specifier} for arg selection.
      } else {
        ret.push(curr);
        textIndex += 1;
      }
    }
    return ret;
  },
  // NOTE: Invalid stream pointers passed to these functions would cause a crash
  //       in native code. We, on the other hand, just ignore them, since it's
  //       easier.
  clearerr__deps: ['$FS'],
  clearerr: function(stream) {
    // void clearerr(FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/clearerr.html
    if (FS.streams[stream]) FS.streams[stream].error = false;
  },
  fclose__deps: ['close', 'fsync'],
  fclose: function(stream) {
    // int fclose(FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fclose.html
    _fsync(stream);
    return _close(stream);
  },
  fdopen__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  fdopen: function(fildes, mode) {
    // FILE *fdopen(int fildes, const char *mode);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fdopen.html
    if (FS.streams[fildes]) {
      var stream = FS.streams[fildes];
      mode = Pointer_stringify(mode);
      if ((mode.indexOf('w') != -1 && !stream.isWrite) ||
          (mode.indexOf('r') != -1 && !stream.isRead) ||
          (mode.indexOf('a') != -1 && !stream.isAppend) ||
          (mode.indexOf('+') != -1 && (!stream.isRead || !stream.isWrite))) {
        ___setErrNo(ERRNO_CODES.EINVAL);
        return 0;
      } else {
        stream.error = false;
        stream.eof = false;
        return fildes;
      }
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return 0;
    }
  },
  feof__deps: ['$FS'],
  feof: function(stream) {
    // int feof(FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/feof.html
    return Number(FS.streams[stream] && FS.streams[stream].eof);
  },
  ferror__deps: ['$FS'],
  ferror: function(stream) {
    // int ferror(FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/ferror.html
    return Number(FS.streams[stream] && FS.streams[stream].error);
  },
  fflush__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  fflush: function(stream) {
    // int fflush(FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fflush.html
    var flush = function(filedes) {
      // Right now we write all data directly, except for output devices.
      if (FS.streams[filedes] && FS.streams[filedes].object.output) {
        if (!FS.streams[filedes].isTerminal) { // don't flush terminals, it would cause a \n to also appear
          FS.streams[filedes].object.output(null);
        }
      }
    };
    try {
      if (stream === 0) {
        for (var i = 0; i < FS.streams.length; i++) if (FS.streams[i]) flush(i);
      } else {
        flush(stream);
      }
      return 0;
    } catch (e) {
      ___setErrNo(ERRNO_CODES.EIO);
      return -1;
    }
  },
  fgetc__deps: ['$FS', 'read'],
  fgetc__postset: '_fgetc.ret = allocate([0], "i8", ALLOC_STATIC);',
  fgetc: function(stream) {
    // int fgetc(FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fgetc.html
    if (!FS.streams[stream]) return -1;
    var streamObj = FS.streams[stream];
    if (streamObj.eof || streamObj.error) return -1;
    var ret = _read(stream, _fgetc.ret, 1);
    if (ret == 0) {
      streamObj.eof = true;
      return -1;
    } else if (ret == -1) {
      streamObj.error = true;
      return -1;
    } else {
      return {{{ makeGetValue('_fgetc.ret', '0', 'i8', null, 1) }}};
    }
  },
  getc: 'fgetc',
  getc_unlocked: 'fgetc',
  getchar__deps: ['fgetc', 'stdin'],
  getchar: function() {
    // int getchar(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getchar.html
    return _fgetc({{{ makeGetValue('_stdin', '0', 'void*') }}});
  },
  fgetpos__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  fgetpos: function(stream, pos) {
    // int fgetpos(FILE *restrict stream, fpos_t *restrict pos);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fgetpos.html
    if (FS.streams[stream]) {
      stream = FS.streams[stream];
      if (stream.object.isDevice) {
        ___setErrNo(ERRNO_CODES.ESPIPE);
        return -1;
      } else {
        {{{ makeSetValue('pos', '0', 'stream.position', 'i32') }}}
        var state = (stream.eof ? 1 : 0) + (stream.error ? 2 : 0);
        {{{ makeSetValue('pos', Runtime.getNativeTypeSize('i32'), 'state', 'i32') }}}
        return 0;
      }
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
  },
  fgets__deps: ['fgetc'],
  fgets: function(s, n, stream) {
    // char *fgets(char *restrict s, int n, FILE *restrict stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fgets.html
    if (!FS.streams[stream]) return 0;
    var streamObj = FS.streams[stream];
    if (streamObj.error || streamObj.eof) return 0;
    var byte_;
    for (var i = 0; i < n - 1 && byte_ != '\n'.charCodeAt(0); i++) {
      byte_ = _fgetc(stream);
      if (byte_ == -1) {
        if (streamObj.error) return 0;
        else if (streamObj.eof) break;
      }
      {{{ makeSetValue('s', 'i', 'byte_', 'i8') }}}
    }
    {{{ makeSetValue('s', 'i', '0', 'i8') }}}
    return s;
  },
  gets__deps: ['fgets'],
  gets: function(s) {
    // char *gets(char *s);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/gets.html
    return _fgets(s, 1e6, {{{ makeGetValue('_stdin', '0', 'void*') }}});
  },
  fileno: function(stream) {
    // int fileno(FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fileno.html
    // We use file descriptor numbers and FILE* streams interchangeably.
    return stream;
  },
  ftrylockfile: function() {
    // int ftrylockfile(FILE *file);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/flockfile.html
    // Locking is useless in a single-threaded environment. Pretend to succeed.
    return 0;
  },
  flockfile: 'ftrylockfile',
  funlockfile: 'ftrylockfile',
  fopen__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', 'open'],
  fopen: function(filename, mode) {
    // FILE *fopen(const char *restrict filename, const char *restrict mode);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fopen.html
    var flags;
    mode = Pointer_stringify(mode);
    if (mode[0] == 'r') {
      if (mode.indexOf('+') != -1) {
        flags = {{{ cDefine('O_RDWR') }}};
      } else {
        flags = {{{ cDefine('O_RDONLY') }}};
      }
    } else if (mode[0] == 'w') {
      if (mode.indexOf('+') != -1) {
        flags = {{{ cDefine('O_RDWR') }}};
      } else {
        flags = {{{ cDefine('O_WRONLY') }}};
      }
      flags |= {{{ cDefine('O_CREAT') }}};
      flags |= {{{ cDefine('O_TRUNC') }}};
    } else if (mode[0] == 'a') {
      if (mode.indexOf('+') != -1) {
        flags = {{{ cDefine('O_RDWR') }}};
      } else {
        flags = {{{ cDefine('O_WRONLY') }}};
      }
      flags |= {{{ cDefine('O_CREAT') }}};
      flags |= {{{ cDefine('O_APPEND') }}};
    } else {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return 0;
    }
    var ret = _open(filename, flags, allocate([0x1FF, 0, 0, 0], 'i32', ALLOC_STACK));  // All creation permissions.
    return (ret == -1) ? 0 : ret;
  },
  fputc__deps: ['$FS', 'write'],
  fputc__postset: '_fputc.ret = allocate([0], "i8", ALLOC_STATIC);',
  fputc: function(c, stream) {
    // int fputc(int c, FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fputc.html
    var chr = unSign(c & 0xFF);
    {{{ makeSetValue('_fputc.ret', '0', 'chr', 'i8') }}}
    var ret = _write(stream, _fputc.ret, 1);
    if (ret == -1) {
      if (FS.streams[stream]) FS.streams[stream].error = true;
      return -1;
    } else {
      return chr;
    }
  },
  putc: 'fputc',
  putc_unlocked: 'fputc',
  putchar__deps: ['fputc', 'stdout'],
  putchar: function(c) {
    // int putchar(int c);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/putchar.html
    return _fputc(c, {{{ makeGetValue('_stdout', '0', 'void*') }}});
  },
  putchar_unlocked: 'putchar',
  fputs__deps: ['write', 'strlen'],
  fputs: function(s, stream) {
    // int fputs(const char *restrict s, FILE *restrict stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fputs.html
    return _write(stream, s, _strlen(s));
  },
  puts__deps: ['fputs', 'fputc', 'stdout'],
  puts: function(s) {
    // int puts(const char *s);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/puts.html
    // NOTE: puts() always writes an extra newline.
    var stdout = {{{ makeGetValue('_stdout', '0', 'void*') }}};
    var ret = _fputs(s, stdout);
    if (ret < 0) {
      return ret;
    } else {
      var newlineRet = _fputc('\n'.charCodeAt(0), stdout);
      return (newlineRet < 0) ? -1 : ret + 1;
    }
  },
  fread__deps: ['$FS', 'read'],
  fread: function(ptr, size, nitems, stream) {
    // size_t fread(void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fread.html
    var bytesToRead = nitems * size;
    if (bytesToRead == 0) return 0;
    var bytesRead = _read(stream, ptr, bytesToRead);
    var streamObj = FS.streams[stream];
    if (bytesRead == -1) {
      if (streamObj) streamObj.error = true;
      return 0;
    } else {
      if (bytesRead < bytesToRead) streamObj.eof = true;
      return Math.floor(bytesRead / size);
    }
  },
  freopen__deps: ['$FS', 'fclose', 'fopen', '__setErrNo', '$ERRNO_CODES'],
  freopen: function(filename, mode, stream) {
    // FILE *freopen(const char *restrict filename, const char *restrict mode, FILE *restrict stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/freopen.html
    if (!filename) {
      if (!FS.streams[stream]) {
        ___setErrNo(ERRNO_CODES.EBADF);
        return 0;
      }
      if (_freopen.buffer) _free(_freopen.buffer);
      filename = intArrayFromString(FS.streams[stream].path);
      filename = allocate(filename, 'i8', ALLOC_NORMAL);
    }
    _fclose(stream);
    return _fopen(filename, mode);
  },
  fseek__deps: ['$FS', 'lseek'],
  fseek: function(stream, offset, whence) {
    // int fseek(FILE *stream, long offset, int whence);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fseek.html
    var ret = _lseek(stream, offset, whence);
    if (ret == -1) {
      return -1;
    } else {
      FS.streams[stream].eof = false;
      return 0;
    }
  },
  fseeko: 'fseek',
  fseeko64: 'fseek',
  fsetpos__deps: ['$FS', 'lseek', '__setErrNo', '$ERRNO_CODES'],
  fsetpos: function(stream, pos) {
    // int fsetpos(FILE *stream, const fpos_t *pos);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fsetpos.html
    if (FS.streams[stream]) {
      if (FS.streams[stream].object.isDevice) {
        ___setErrNo(ERRNO_CODES.EPIPE);
        return -1;
      } else {
        FS.streams[stream].position = {{{ makeGetValue('pos', '0', 'i32') }}};
        var state = {{{ makeGetValue('pos', Runtime.getNativeTypeSize('i32'), 'i32') }}};
        FS.streams[stream].eof = Boolean(state & 1);
        FS.streams[stream].error = Boolean(state & 2);
        return 0;
      }
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
  },
  ftell__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  ftell: function(stream) {
    // long ftell(FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/ftell.html
    if (FS.streams[stream]) {
      stream = FS.streams[stream];
      if (stream.object.isDevice) {
        ___setErrNo(ERRNO_CODES.ESPIPE);
        return -1;
      } else {
        return stream.position;
      }
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
  },
  ftello: 'ftell',
  ftello64: 'ftell',
  fwrite__deps: ['$FS', 'write'],
  fwrite: function(ptr, size, nitems, stream) {
    // size_t fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/fwrite.html
    var bytesToWrite = nitems * size;
    if (bytesToWrite == 0) return 0;
    var bytesWritten = _write(stream, ptr, bytesToWrite);
    if (bytesWritten == -1) {
      if (FS.streams[stream]) FS.streams[stream].error = true;
      return 0;
    } else {
      return Math.floor(bytesWritten / size);
    }
  },
  popen__deps: ['__setErrNo', '$ERRNO_CODES'],
  popen: function(command, mode) {
    // FILE *popen(const char *command, const char *mode);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/popen.html
    // We allow only one process, so no pipes.
    ___setErrNo(ERRNO_CODES.EMFILE);
    return 0;
  },
  pclose__deps: ['__setErrNo', '$ERRNO_CODES'],
  pclose: function(stream) {
    // int pclose(FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/pclose.html
    // We allow only one process, so no pipes.
    ___setErrNo(ERRNO_CODES.ECHILD);
    return -1;
  },
  perror__deps: ['puts', 'putc', 'strerror', '__errno_location'],
  perror: function(s) {
    // void perror(const char *s);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/perror.html
    if (s) {
      _puts(s);
      _putc(':'.charCodeAt(0));
      _putc(' '.charCodeAt(0));
    }
    var errnum = {{{ makeGetValue('___errno_location()', '0', 'i32') }}};
    _puts(_strerror(errnum));
  },
  remove__deps: ['unlink', 'rmdir'],
  remove: function(path) {
    // int remove(const char *path);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/remove.html
    var ret = _unlink(path);
    if (ret == -1) ret = _rmdir(path);
    return ret;
  },
  rename__deps: ['__setErrNo', '$ERRNO_CODES'],
  rename: function(old, new_) {
    // int rename(const char *old, const char *new);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/rename.html
    var oldObj = FS.analyzePath(Pointer_stringify(old));
    var newObj = FS.analyzePath(Pointer_stringify(new_));
    if (newObj.path == oldObj.path) {
      return 0;
    } else if (!oldObj.exists) {
      ___setErrNo(oldObj.error);
      return -1;
    } else if (oldObj.isRoot || oldObj.path == FS.currentPath) {
      ___setErrNo(ERRNO_CODES.EBUSY);
      return -1;
    } else if (newObj.path && newObj.path.indexOf(oldObj.path) == 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else if (newObj.exists && newObj.object.isFolder) {
      ___setErrNo(ERRNO_CODES.EISDIR);
      return -1;
    } else {
      delete oldObj.parentObject.contents[oldObj.name];
      newObj.parentObject.contents[newObj.name] = oldObj.object;
      return 0;
    }
  },
  rewind__deps: ['$FS', 'fseek'],
  rewind: function(stream) {
    // void rewind(FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/rewind.html
    _fseek(stream, 0, 0);  // SEEK_SET.
    if (FS.streams[stream]) FS.streams[stream].error = false;
  },
  setvbuf: function(stream, buf, type, size) {
    // int setvbuf(FILE *restrict stream, char *restrict buf, int type, size_t size);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/setvbuf.html
    // TODO: Implement custom buffering.
    return 0;
  },
  setbuf__deps: ['setvbuf'],
  setbuf: function(stream, buf) {
    // void setbuf(FILE *restrict stream, char *restrict buf);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/setbuf.html
    if (buf) _setvbuf(stream, buf, 0, 8192);  // _IOFBF, BUFSIZ.
    else _setvbuf(stream, buf, 2, 8192);  // _IONBF, BUFSIZ.
  },
  tmpnam__deps: ['$FS'],
  tmpnam: function(s, dir, prefix) {
    // char *tmpnam(char *s);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/tmpnam.html
    // NOTE: The dir and prefix arguments are for internal use only.
    var folder = FS.findObject(dir || '/tmp');
    if (!folder || !folder.isFolder) {
      dir = '/tmp';
      folder = FS.findObject(dir);
      if (!folder || !folder.isFolder) return 0;
    }
    var name = prefix || 'file';
    do {
      name += String.fromCharCode(65 + Math.floor(Math.random() * 25));
    } while (name in folder.contents);
    var result = dir + '/' + name;
    if (!_tmpnam.buffer) _tmpnam.buffer = _malloc(256);
    if (!s) s = _tmpnam.buffer;
    for (var i = 0; i < result.length; i++) {
      {{{ makeSetValue('s', 'i', 'result.charCodeAt(i)', 'i8') }}};
    }
    {{{ makeSetValue('s', 'i', '0', 'i8') }}};
    return s;
  },
  tempnam__deps: ['tmpnam'],
  tempnam: function(dir, pfx) {
    // char *tempnam(const char *dir, const char *pfx);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/tempnam.html
    return _tmpnam(0, Pointer_stringify(dir), Pointer_stringify(pfx));
  },
  tmpfile__deps: ['tmpnam', 'fopen'],
  tmpfile: function() {
    // FILE *tmpfile(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/tmpfile.html
    // TODO: Delete the created file on closing.
    if (_tmpfile.mode) {
      _tmpfile.mode = allocate(intArrayFromString('w+'), 'i8', ALLOC_NORMAL);
    }
    return _fopen(_tmpnam(0), _tmpfile.mode);
  },
  ungetc__deps: ['$FS'],
  ungetc: function(c, stream) {
    // int ungetc(int c, FILE *stream);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/ungetc.html
    if (FS.streams[stream]) {
      c = unSign(c & 0xFF);
      FS.streams[stream].ungotten.push(c);
      return c;
    } else {
      return -1;
    }
  },
  system__deps: ['__setErrNo', '$ERRNO_CODES'],
  system: function(command) {
    // int system(const char *command);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/system.html
    // Can't call external programs.
    ___setErrNo(ERRNO_CODES.EAGAIN);
    return -1;
  },
  fscanf__deps: ['$FS', '__setErrNo', '$ERRNO_CODES',
                 '_scanString', 'getc', 'ungetc'],
  fscanf: function(stream, format, varargs) {
    // int fscanf(FILE *restrict stream, const char *restrict format, ... );
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/scanf.html
    if (FS.streams[stream]) {
      var stack = [];
      var get = function() { var ret = _fgetc(stream); stack.push(ret); return ret };
      var unget = function(c) { return _ungetc(stack.pop(), stream) };
      return __scanString(format, get, unget, varargs);
    } else {
      return -1;
    }
  },
  scanf__deps: ['fscanf'],
  scanf: function(format, varargs) {
    // int scanf(const char *restrict format, ... );
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/scanf.html
    var stdin = {{{ makeGetValue('_stdin', '0', 'void*') }}};
    return _fscanf(stdin, format, varargs);
  },
  sscanf__deps: ['_scanString'],
  sscanf: function(s, format, varargs) {
    // int sscanf(const char *restrict s, const char *restrict format, ... );
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/scanf.html
    var index = 0;
    var get = function() { return {{{ makeGetValue('s', 'index++', 'i8') }}}; };
    var unget = function() { index--; };
    return __scanString(format, get, unget, varargs);
  },
  snprintf__deps: ['_formatString'],
  snprintf: function(s, n, format, varargs) {
    // int snprintf(char *restrict s, size_t n, const char *restrict format, ...);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/printf.html
    var result = __formatString(format, varargs);
    var limit = (n === undefined) ? result.length
                                  : Math.min(result.length, Math.max(n - 1, 0));
    for (var i = 0; i < limit; i++) {
      {{{ makeSetValue('s', 'i', 'result[i]', 'i8') }}};
    }
    if (limit < n || (n === undefined)) {{{ makeSetValue('s', 'i', '0', 'i8') }}};
    return result.length;
  },
  fprintf__deps: ['fwrite', '_formatString'],
  fprintf: function(stream, format, varargs) {
    // int fprintf(FILE *restrict stream, const char *restrict format, ...);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/printf.html
    var result = __formatString(format, varargs);
    var stack = Runtime.stackSave();
    var ret = _fwrite(allocate(result, 'i8', ALLOC_STACK), 1, result.length, stream);
    Runtime.stackRestore(stack);
    return ret;
  },
  printf__deps: ['fprintf'],
  printf: function(format, varargs) {
    // int printf(const char *restrict format, ...);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/printf.html
    var stdout = {{{ makeGetValue('_stdout', '0', 'void*') }}};
    return _fprintf(stdout, format, varargs);
  },
  sprintf__deps: ['snprintf'],
  sprintf: function(s, format, varargs) {
    // int sprintf(char *restrict s, const char *restrict format, ...);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/printf.html
    return _snprintf(s, undefined, format, varargs);
  },
  vfprintf: 'fprintf',
  vsnprintf: 'snprintf',
  vprintf: 'printf',
  vsprintf: 'sprintf',
  vscanf: 'scanf',
  vfscanf: 'fscanf',
  vsscanf: 'sscanf',
  fopen64: 'fopen',
  __01fopen64_: 'fopen',
  __01freopen64_: 'freopen',
  __01fseeko64_: 'fseek',
  __01ftello64_: 'ftell',
  __01tmpfile64_: 'tmpfile',
  __isoc99_fscanf: 'fscanf',
  // TODO: Check if any other aliases are needed.
  _IO_getc: 'getc',
  _IO_putc: 'putc',
  _ZNSo3putEc: 'putchar',
  _ZNSo5flushEv__deps: ['fflush', 'stdout'],
  _ZNSo5flushEv: function() {
    _fflush({{{ makeGetValue('_stdout', '0', 'void*') }}});
  },

  // ==========================================================================
  // sys/mman.h
  // ==========================================================================

  mmap__deps: ['$FS'],
  mmap: function(start, num, prot, flags, stream, offset) {
    /* FIXME: Since mmap is normally implemented at the kernel level,
     * this implementation simply uses malloc underneath the call to
     * mmap.
     */
    if (!_mmap.mappings) _mmap.mappings = {};
    if (stream == -1) {
      var ptr = _malloc(num);
    } else {
      var info = FS.streams[stream];
      if (!info) return -1;
      var contents = info.object.contents;
      contents = Array.prototype.slice.call(contents, offset, offset+num);
      ptr = allocate(contents, 'i8', ALLOC_NORMAL);
    }
    // align to page size
    var ret = ptr;
    if (ptr % PAGE_SIZE != 0) {
      var old = ptr;
      ptr = _malloc(num + PAGE_SIZE);
      ret = alignMemoryPage(ptr);
      _memcpy(ret, old, num);
      _free(old);
    }
    if (stream == -1) {
      _memset(ret, 0, num);
    }
    _mmap.mappings[ret] = { malloc: ptr, num: num };
    return ret;
  },
  __01mmap64_: 'mmap',

  munmap: function(start, num) {
    if (!_mmap.mappings) _mmap.mappings = {};
    // TODO: support unmmap'ing parts of allocations
    var info = _mmap.mappings[start];
    if (!info) return 0;
    if (num == info.num) {
      _mmap.mappings[start] = null;
      _free(info.malloc);
    }
    return 0;
  },

  // TODO: Implement mremap.

  // ==========================================================================
  // stdlib.h
  // ==========================================================================

  malloc: function(bytes) {
    /* Over-allocate to make sure it is byte-aligned by 8.
     * This will leak memory, but this is only the dummy
     * implementation (replaced by dlmalloc normally) so
     * not an issue.
     */
    ptr = Runtime.staticAlloc(bytes + 8);
    return (ptr+8) & 0xFFFFFFF8;
  },
  _Znwj: 'malloc',
  _Znaj: 'malloc',
  _Znam: 'malloc',
  _Znwm: 'malloc',

  free: function(){},
  _ZdlPv: 'free',
  _ZdaPv: 'free',

  calloc__deps: ['malloc'],
  calloc: function(n, s) {
    var ret = _malloc(n*s);
    _memset(ret, 0, n*s);
    return ret;
  },

  abs: 'Math.abs',
  labs: 'Math.abs',

  exit__deps: ['_exit'],
  exit: function(status) {
    __exit(status);
  },

  atexit: function(func, arg) {
    __ATEXIT__.unshift({ func: func, arg: arg });
  },
  __cxa_atexit: 'atexit',

  abort: function() {
    ABORT = true;
    throw 'abort() at ' + (new Error().stack);
  },

  bsearch: function(key, base, num, size, compar) {
    var cmp = FUNCTION_TABLE[compar];
    var left = 0;
    var right = num;
    var mid, test, addr;

    while (left < right) {
      mid = (left + right) >>> 1;
      addr = base + (mid * size);
      test = cmp(key, addr);

      if (test < 0) {
        right = mid;
      } else if (test > 0) {
        left = mid + 1;
      } else {
        return addr;
      }
    }

    return 0;
  },

  realloc__deps: ['memcpy'],
  realloc: function(ptr, size) {
    // Very simple, inefficient implementation - if you use a real malloc, best to use
    // a real realloc with it
    if (!size) {
      if (ptr) _free(ptr);
      return 0;
    }
    var ret = _malloc(size);
    if (ptr) {
      _memcpy(ret, ptr, size); // might be some invalid reads
      _free(ptr);
    }
    return ret;
  },

  strtod__deps: ['isspace', 'isdigit'],
  strtod: function(str, endptr) {
    var origin = str;

    // Skip space.
    while (_isspace({{{ makeGetValue('str', 0, 'i8') }}})) str++;

    // Check for a plus/minus sign.
    var multiplier = 1;
    if ({{{ makeGetValue('str', 0, 'i8') }}} == '-'.charCodeAt(0)) {
      multiplier = -1;
      str++;
    } else if ({{{ makeGetValue('str', 0, 'i8') }}} == '+'.charCodeAt(0)) {
      str++;
    }

    var chr;
    var ret = 0;

    // Get whole part.
    var whole = false;
    while(1) {
      chr = {{{ makeGetValue('str', 0, 'i8') }}};
      if (!_isdigit(chr)) break;
      whole = true;
      ret = ret*10 + chr - '0'.charCodeAt(0);
      str++;
    }

    // Get fractional part.
    var fraction = false;
    if ({{{ makeGetValue('str', 0, 'i8') }}} == '.'.charCodeAt(0)) {
      str++;
      var mul = 1/10;
      while(1) {
        chr = {{{ makeGetValue('str', 0, 'i8') }}};
        if (!_isdigit(chr)) break;
        fraction = true;
        ret += mul*(chr - '0'.charCodeAt(0));
        mul /= 10;
        str++;
      }
    }

    if (!whole && !fraction) {
      if (endptr) {
        {{{ makeSetValue('endptr', 0, 'origin', '*') }}}
      }
      return 0;
    }

    // Get exponent part.
    chr = {{{ makeGetValue('str', 0, 'i8') }}};
    if (chr == 'e'.charCodeAt(0) || chr == 'E'.charCodeAt(0)) {
      str++;
      var exponent = 0;
      var expNegative = false;
      chr = {{{ makeGetValue('str', 0, 'i8') }}};
      if (chr == '-'.charCodeAt(0)) {
        expNegative = true;
        str++;
      } else if (chr == '+'.charCodeAt(0)) {
        str++;
      }
      chr = {{{ makeGetValue('str', 0, 'i8') }}};
      while(1) {
        if (!_isdigit(chr)) break;
        exponent = exponent*10 + chr - '0'.charCodeAt(0);
        str++;
        chr = {{{ makeGetValue('str', 0, 'i8') }}};
      }
      if (expNegative) exponent = -exponent;
      ret *= Math.pow(10, exponent);
    }

    // Set end pointer.
    if (endptr) {
      {{{ makeSetValue('endptr', 0, 'str', '*') }}}
    }

    return ret * multiplier;
  },
  strtod_l: 'strtod', // no locale support yet
  strtold: 'strtod', // XXX add real support for long double
  strtold_l: 'strtold', // no locale support yet
  strtof: 'strtod', // use stdtod to handle strtof

  _parseInt__deps: ['isspace', '__setErrNo', '$ERRNO_CODES'],
  _parseInt: function(str, endptr, base, min, max, bits, unsign) {
    // Skip space.
    while (_isspace({{{ makeGetValue('str', 0, 'i8') }}})) str++;

    // Check for a plus/minus sign.
    var multiplier = 1;
    if ({{{ makeGetValue('str', 0, 'i8') }}} == '-'.charCodeAt(0)) {
      multiplier = -1;
      str++;
    } else if ({{{ makeGetValue('str', 0, 'i8') }}} == '+'.charCodeAt(0)) {
      str++;
    }

    // Find base.
    var finalBase = base;
    if (!finalBase) {
      if ({{{ makeGetValue('str', 0, 'i8') }}} == '0'.charCodeAt(0)) {
        if ({{{ makeGetValue('str+1', 0, 'i8') }}} == 'x'.charCodeAt(0) ||
            {{{ makeGetValue('str+1', 0, 'i8') }}} == 'X'.charCodeAt(0)) {
          finalBase = 16;
          str += 2;
        } else {
          finalBase = 8;
          str++;
        }
      }
    }
    if (!finalBase) finalBase = 10;

    // Get digits.
    var chr;
    var ret = 0;
    while ((chr = {{{ makeGetValue('str', 0, 'i8') }}}) != 0) {
      var digit = parseInt(String.fromCharCode(chr), finalBase);
      if (isNaN(digit)) {
        break;
      } else {
        ret = ret * finalBase + digit;
        str++;
      }
    }

    // Apply sign.
    ret *= multiplier;

    // Set end pointer.
    if (endptr) {
      {{{ makeSetValue('endptr', 0, 'str', '*') }}}
    }

    // Unsign if needed.
    if (unsign) {
      if (Math.abs(ret) > max) {
        ret = max;
        ___setErrNo(ERRNO_CODES.ERANGE);
      } else {
        ret = unSign(ret, bits);
      }
    }

    // Validate range.
    if (ret > max || ret < min) {
      ret = ret > max ? max : min;
      ___setErrNo(ERRNO_CODES.ERANGE);
    }

#if USE_TYPED_ARRAYS == 2
    if (bits == 64) {
      ret = [{{{ splitI64('ret') }}}];
    }
#endif

    return ret;
  },
#if USE_TYPED_ARRAYS == 2
  _parseInt64__deps: ['isspace', '__setErrNo', '$ERRNO_CODES', function() { Types.preciseI64MathUsed = 1 }],
  _parseInt64: function(str, endptr, base, min, max, unsign) {
    var start = str;
    // Skip space.
    while (_isspace({{{ makeGetValue('str', 0, 'i8') }}})) str++;

    // Check for a plus/minus sign.
    if ({{{ makeGetValue('str', 0, 'i8') }}} == '-'.charCodeAt(0)) {
      str++;
    } else if ({{{ makeGetValue('str', 0, 'i8') }}} == '+'.charCodeAt(0)) {
      str++;
    }

    // Find base.
    var ok = false;
    var finalBase = base;
    if (!finalBase) {
      if ({{{ makeGetValue('str', 0, 'i8') }}} == '0'.charCodeAt(0)) {
        if ({{{ makeGetValue('str+1', 0, 'i8') }}} == 'x'.charCodeAt(0) ||
            {{{ makeGetValue('str+1', 0, 'i8') }}} == 'X'.charCodeAt(0)) {
          finalBase = 16;
          str += 2;
        } else {
          finalBase = 8;
          str++;
          ok = true; // we saw an initial zero, perhaps the entire thing is just "0"
        }
      }
    }
    if (!finalBase) finalBase = 10;

    // Get digits.
    var chr;
    while ((chr = {{{ makeGetValue('str', 0, 'i8') }}}) != 0) {
      var digit = parseInt(String.fromCharCode(chr), finalBase);
      if (isNaN(digit)) {
        break;
      } else {
        str++;
        ok = true;
      }
    }
    if (!ok) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return [0, 0];
    }

    try {
      i64Math.fromString(Pointer_stringify(start, str - start), finalBase, min, max, unsign);
    } catch(e) {
      ___setErrNo(ERRNO_CODES.ERANGE); // not quite correct
    }

    // Set end pointer.
    if (endptr) {
      {{{ makeSetValue('endptr', 0, 'str', '*') }}}
    }

    var ret = i64Math.result.slice(0);

    return ret;
  },
#endif
  strtoll__deps: ['_parseInt64'],
  strtoll: function(str, endptr, base) {
    return __parseInt64(str, endptr, base, '-9223372036854775808', '9223372036854775807');  // LLONG_MIN, LLONG_MAX.
  },
  strtoll_l: 'strtoll', // no locale support yet
  strtol__deps: ['_parseInt'],
  strtol: function(str, endptr, base) {
    return __parseInt(str, endptr, base, -2147483648, 2147483647, 32);  // LONG_MIN, LONG_MAX.
  },
  strtol_l: 'strtol', // no locale support yet
  strtoul__deps: ['_parseInt'],
  strtoul: function(str, endptr, base) {
    return __parseInt(str, endptr, base, 0, 4294967295, 32, true);  // ULONG_MAX.
  },
  strtoul_l: 'strtoul', // no locale support yet
  strtoull__deps: ['_parseInt64'],
  strtoull: function(str, endptr, base) {
    return __parseInt64(str, endptr, base, 0, '18446744073709551615', true);  // ULONG_MAX.
  },
  strtoull_l: 'strtoull', // no locale support yet

  atof__deps: ['strtod'],
  atof: function(ptr) {
    return _strtod(ptr, null);
  },

  atoi__deps: ['strtol'],
  atoi: function(ptr) {
    return _strtol(ptr, null, 10);
  },
  atol: 'atoi',

  atoll__deps: ['strtoll'],
  atoll: function(ptr) {
    return _strtoll(ptr, null, 10);
  },

  qsort__deps: ['memcpy'],
  qsort: function(base, num, size, comparator) {
    if (num == 0 || size == 0) return;
    // forward calls to the JavaScript sort method
    // first, sort the items logically
    comparator = FUNCTION_TABLE[comparator];
    var keys = [];
    for (var i = 0; i < num; i++) keys.push(i);
    keys.sort(function(a, b) {
      return comparator(base+a*size, base+b*size);
    });
    // apply the sort
    var temp = _malloc(num*size);
    _memcpy(temp, base, num*size);
    for (var i = 0; i < num; i++) {
      if (keys[i] == i) continue; // already in place
      _memcpy(base+i*size, temp+keys[i]*size, size);
    }
    _free(temp);
  },

  environ: null,
  __environ: null,
  __buildEnvironment__deps: ['environ', '__environ'],
  __buildEnvironment: function(env) {
    // WARNING: Arbitrary limit!
    var MAX_ENV_VALUES = 64;
    var TOTAL_ENV_SIZE = 1024;

    // Statically allocate memory for the environment.
    var poolPtr;
    var envPtr;
    if (_environ === null) {
      // Set default values. Use string keys for Closure Compiler compatibility.
      ENV['USER'] = 'root';
      ENV['PATH'] = '/';
      ENV['PWD'] = '/';
      ENV['HOME'] = '/home/emscripten';
      ENV['LANG'] = 'en_US.UTF-8';
      ENV['_'] = './this.program';
      // Allocate memory.
      poolPtr = allocate(TOTAL_ENV_SIZE, 'i8', ALLOC_STATIC);
      envPtr = allocate(MAX_ENV_VALUES * {{{ Runtime.QUANTUM_SIZE }}},
                        'i8*', ALLOC_STATIC);
      {{{ makeSetValue('envPtr', '0', 'poolPtr', 'i8*') }}}
      _environ = allocate([envPtr], 'i8**', ALLOC_STATIC);
      // Set up global variable alias.
      ___environ = _environ;
    } else {
      envPtr = {{{ makeGetValue('_environ', '0', 'i8**') }}};
      poolPtr = {{{ makeGetValue('envPtr', '0', 'i8*') }}};
    }

    // Collect key=value lines.
    var strings = [];
    var totalSize = 0;
    for (var key in env) {
      if (typeof env[key] === 'string') {
        var line = key + '=' + env[key];
        strings.push(line);
        totalSize += line.length;
      }
    }
    if (totalSize > TOTAL_ENV_SIZE) {
      throw new Error('Environment size exceeded TOTAL_ENV_SIZE!');
    }

    // Make new.
    var ptrSize = {{{ Runtime.getNativeTypeSize('i8*') }}};
    for (var i = 0; i < strings.length; i++) {
      var line = strings[i];
      for (var j = 0; j < line.length; j++) {
        {{{ makeSetValue('poolPtr', 'j', 'line.charCodeAt(j)', 'i8') }}};
      }
      {{{ makeSetValue('poolPtr', 'j', '0', 'i8') }}};
      {{{ makeSetValue('envPtr', 'i * ptrSize', 'poolPtr', 'i8*') }}};
      poolPtr += line.length + 1;
    }
    {{{ makeSetValue('envPtr', 'strings.length * ptrSize', '0', 'i8*') }}};
  },
  $ENV__deps: ['__buildEnvironment'],
  $ENV__postset: '___buildEnvironment(ENV);',
  $ENV: {},
  getenv__deps: ['$ENV'],
  getenv: function(name) {
    // char *getenv(const char *name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/getenv.html
    if (name === 0) return 0;
    name = Pointer_stringify(name);
    if (!ENV.hasOwnProperty(name)) return 0;

    if (_getenv.ret) _free(_getenv.ret);
    _getenv.ret = allocate(intArrayFromString(ENV[name]), 'i8', ALLOC_NORMAL);
    return _getenv.ret;
  },
  clearenv__deps: ['$ENV', '__buildEnvironment'],
  clearenv: function(name) {
    // int clearenv (void);
    // http://www.gnu.org/s/hello/manual/libc/Environment-Access.html#index-clearenv-3107
    ENV = {};
    ___buildEnvironment(ENV);
    return 0;
  },
  setenv__deps: ['$ENV', '__buildEnvironment', '$ERRNO_CODES', '__setErrNo'],
  setenv: function(envname, envval, overwrite) {
    // int setenv(const char *envname, const char *envval, int overwrite);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/setenv.html
    if (envname === 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    var name = Pointer_stringify(envname);
    var val = Pointer_stringify(envval);
    if (name === '' || name.indexOf('=') !== -1) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    if (ENV.hasOwnProperty(name) && !overwrite) return 0;
    ENV[name] = val;
    ___buildEnvironment(ENV);
    return 0;
  },
  unsetenv__deps: ['$ENV', '__buildEnvironment', '$ERRNO_CODES', '__setErrNo'],
  unsetenv: function(name) {
    // int unsetenv(const char *name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/unsetenv.html
    if (name === 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    name = Pointer_stringify(name);
    if (name === '' || name.indexOf('=') !== -1) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    if (ENV.hasOwnProperty(name)) {
      delete ENV[name];
      ___buildEnvironment(ENV);
    }
    return 0;
  },
  putenv__deps: ['$ENV', '__buildEnvironment', '$ERRNO_CODES', '__setErrNo'],
  putenv: function(string) {
    // int putenv(char *string);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/putenv.html
    // WARNING: According to the standard (and the glibc implementation), the
    //          string is taken by reference so future changes are reflected.
    //          We copy it instead, possibly breaking some uses.
    if (string === 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    string = Pointer_stringify(string);
    var splitPoint = string.indexOf('=')
    if (string === '' || string.indexOf('=') === -1) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    }
    var name = string.slice(0, splitPoint);
    var value = string.slice(splitPoint + 1);
    if (!(name in ENV) || ENV[name] !== value) {
      ENV[name] = value;
      ___buildEnvironment(ENV);
    }
    return 0;
  },

  getloadavg: function(loadavg, nelem) {
    // int getloadavg(double loadavg[], int nelem);
    // http://linux.die.net/man/3/getloadavg
    var limit = Math.min(nelem, 3);
    var doubleSize = {{{ Runtime.getNativeTypeSize('double') }}};
    for (var i = 0; i < limit; i++) {
      {{{ makeSetValue('loadavg', 'i * doubleSize', '0.1', 'double') }}}
    }
    return limit;
  },

  // Use browser's Math.random(). We can't set a seed, though.
  srand: function(seed) {}, // XXX ignored
  rand: function() {
    return Math.floor(Math.random()*0x80000000);
  },
  rand_r: function(seed) { // XXX ignores the seed
    return Math.floor(Math.random()*0x80000000);
  },

  drand48: function() {
    return Math.random();
  },

  realpath__deps: ['$FS', '__setErrNo'],
  realpath: function(file_name, resolved_name) {
    // char *realpath(const char *restrict file_name, char *restrict resolved_name);
    // http://pubs.opengroup.org/onlinepubs/009604499/functions/realpath.html
    var absolute = FS.analyzePath(Pointer_stringify(file_name));
    if (absolute.error) {
      ___setErrNo(absolute.error);
      return 0;
    } else {
      var size = Math.min(4095, absolute.path.length);  // PATH_MAX - 1.
      for (var i = 0; i < size; i++) {
        {{{ makeSetValue('resolved_name', 'i', 'absolute.path.charCodeAt(i)', 'i8') }}}
      }
      {{{ makeSetValue('resolved_name', 'size', '0', 'i8') }}}
      return resolved_name;
    }
  },

  mbtowc: function(pwc, pmb, maxx) {
    // XXX doesn't really handle multibyte at all
    if (!pmb) return 0;
    maxx = Math.min({{{ cDefine('_NL_CTYPE_MB_CUR_MAX') }}}, maxx);
    var i;
    for (i = 0; i < maxx; i++) {
      var curr = {{{ makeGetValue('pmb', 0, 'i8') }}};
      if (pwc) {
        {{{ makeSetValue('pwc', '0', 'curr', 'i8') }}};
        {{{ makeSetValue('pwc', '1', '0', 'i8') }}};
        pwc += 2;
      }
      pmb++;
      if (!curr) break;
    }
    return i;
  },

  wcrtomb: function(s, wc, ps) {
    // XXX doesn't really handle multibyte at all
    if (s) {
      {{{ makeSetValue('s', '0', 'wc', 'i8') }}};
    }
    return 1;
  },

  // ==========================================================================
  // string.h
  // ==========================================================================

  // FIXME: memcpy, memmove and memset should all return their destination pointers.

  memcpy__inline: function (dest, src, num, align) {
    var ret = '';
#if ASSERTIONS
    ret += "assert(" + num + " % 1 === 0, 'memcpy given ' + " + num + " + ' bytes to copy. Problem with quantum=1 corrections perhaps?');";
#endif
    ret += makeCopyValues(dest, src, num, 'null', null, align);
    return ret;
  },
  memcpy: function (dest, src, num, align) {
#if ASSERTIONS
    assert(num % 1 === 0, 'memcpy given ' + num + ' bytes to copy. Problem with quantum=1 corrections perhaps?');
#endif
#if USE_TYPED_ARRAYS == 2
    if (num >= {{{ SEEK_OPTIMAL_ALIGN_MIN }}} && src % 2 == dest % 2) {
      // This is unaligned, but quite large, and potentially alignable, so work hard to get to aligned settings
      if (src % 4 == dest % 4) {
        var stop = src + num;
        while (src % 4) { // no need to check for stop, since we have large num
          HEAP8[dest++] = HEAP8[src++];
        }
        var src4 = src >> 2, dest4 = dest >> 2, stop4 = stop >> 2;
        while (src4 < stop4) {
          HEAP32[dest4++] = HEAP32[src4++];
        }
        src = src4 << 2;
        dest = dest4 << 2;
        while (src < stop) {
          HEAP8[dest++] = HEAP8[src++];
        }
      } else {
        var stop = src + num;
        if (src % 2) { // no need to check for stop, since we have large num
          HEAP8[dest++] = HEAP8[src++];
        }
        var src2 = src >> 1, dest2 = dest >> 1, stop2 = stop >> 1;
        while (src2 < stop2) {
          HEAP16[dest2++] = HEAP16[src2++];
        }
        src = src2 << 1;
        dest = dest2 << 1;
        if (src < stop) {
          HEAP8[dest++] = HEAP8[src++];
        }
      }
    } else {
      while (num--) {
        HEAP8[dest++] = HEAP8[src++];
      }
    }
#else
    {{{ makeCopyValues('dest', 'src', 'num', 'null', null, 'align') }}};
#endif
  },

  llvm_memcpy_i32: 'memcpy',
  llvm_memcpy_i64: 'memcpy',
  llvm_memcpy_p0i8_p0i8_i32: 'memcpy',
  llvm_memcpy_p0i8_p0i8_i64: 'memcpy',

  memmove__deps: ['memcpy'],
  memmove: function(dest, src, num, align) {
    if (src < dest && dest < src + num) {
      // Copy backwards in a safe manner
      src += num;
      dest += num;
      while (num--) {
        dest--;
        src--;
        {{{ makeCopyValues('dest', 'src', 1, 'null', null, 1) }}};
      }
    } else {
      _memcpy(dest, src, num, align);
    }
  },
  llvm_memmove_i32: 'memmove',
  llvm_memmove_i64: 'memmove',
  llvm_memmove_p0i8_p0i8_i32: 'memmove',
  llvm_memmove_p0i8_p0i8_i64: 'memmove',

  memset__inline: function(ptr, value, num, align) {
    return makeSetValues(ptr, 0, value, 'null', num, align);
  },
  memset: function(ptr, value, num, align) {
#if USE_TYPED_ARRAYS == 2
    // TODO: make these settings, and in memcpy, {{'s
    if (num >= {{{ SEEK_OPTIMAL_ALIGN_MIN }}}) {
      // This is unaligned, but quite large, so work hard to get to aligned settings
      var stop = ptr + num;
      while (ptr % 4) { // no need to check for stop, since we have large num
        HEAP8[ptr++] = value;
      }
      if (value < 0) value += 256; // make it unsigned
      var ptr4 = ptr >> 2, stop4 = stop >> 2, value4 = value | (value << 8) | (value << 16) | (value << 24);
      while (ptr4 < stop4) {
        HEAP32[ptr4++] = value4;
      }
      ptr = ptr4 << 2;
      while (ptr < stop) {
        HEAP8[ptr++] = value;
      }
    } else {
      while (num--) {
        HEAP8[ptr++] = value;
      }
    }
#else
    {{{ makeSetValues('ptr', '0', 'value', 'null', 'num', 'align') }}};
#endif
  },
  llvm_memset_i32: 'memset',
  llvm_memset_p0i8_i32: 'memset',
  llvm_memset_p0i8_i64: 'memset',

  strlen: function(ptr) {
    return String_len(ptr);
  },

  // TODO: Implement when we have real unicode support.
  mblen: function() {
    return 1;
  },

  strspn: function(pstr, pset) {
    var str = pstr, set, strcurr, setcurr;
    while (1) {
      strcurr = {{{ makeGetValue('str', '0', 'i8') }}};
      if (!strcurr) return str - pstr;
      set = pset;
      while (1) {
        setcurr = {{{ makeGetValue('set', '0', 'i8') }}};
        if (!setcurr || setcurr == strcurr) break;
        set++;
      }
      if (!setcurr) return str - pstr;
      str++;
    }
  },

  strcspn: function(pstr, pset) {
    var str = pstr, set, strcurr, setcurr;
    while (1) {
      strcurr = {{{ makeGetValue('str', '0', 'i8') }}};
      if (!strcurr) return str - pstr;
      set = pset;
      while (1) {
        setcurr = {{{ makeGetValue('set', '0', 'i8') }}};
        if (!setcurr || setcurr == strcurr) break;
        set++;
      }
      if (setcurr) return str - pstr;
      str++;
    }
  },

  strcpy: function(pdest, psrc) {
    var i = 0;
    do {
      {{{ makeCopyValues('pdest+i', 'psrc+i', 1, 'i8', null, 1) }}};
      i ++;
    } while ({{{ makeGetValue('psrc', 'i-1', 'i8') }}} != 0);
    return pdest;
  },
  stpcpy: function(pdest, psrc) {
    var i = 0;
    do {
      {{{ makeCopyValues('pdest+i', 'psrc+i', 1, 'i8', null, 1) }}};
      i ++;
    } while ({{{ makeGetValue('psrc', 'i-1', 'i8') }}} != 0);
    return pdest + i - 1;
  },

  strncpy: function(pdest, psrc, num) {
    var padding = false, curr;
    for (var i = 0; i < num; i++) {
      curr = padding ? 0 : {{{ makeGetValue('psrc', 'i', 'i8') }}};
      {{{ makeSetValue('pdest', 'i', 'curr', 'i8') }}}
      padding = padding || {{{ makeGetValue('psrc', 'i', 'i8') }}} == 0;
    }
    return pdest;
  },
  
  strlwr__deps:['tolower'],
  strlwr: function(pstr){
    var i = 0;
    while(1) {
      var x = {{{ makeGetValue('pstr', 'i', 'i8') }}};
      if(x == 0) break;
      {{{ makeSetValue('pstr', 'i', '_tolower(x)', 'i8') }}};
      i++;
    }
  },
  
  strupr__deps:['toupper'],
  strupr: function(pstr){
    var i = 0;
    while(1) {
      var x = {{{ makeGetValue('pstr', 'i', 'i8') }}};
      if(x == 0) break;
      {{{ makeSetValue('pstr', 'i', '_toupper(x)', 'i8') }}};
      i++;
    }
  },

  strcat__deps: ['strlen'],
  strcat: function(pdest, psrc) {
    var len = _strlen(pdest);
    var i = 0;
    do {
      {{{ makeCopyValues('pdest+len+i', 'psrc+i', 1, 'i8', null, 1) }}};
      i ++;
    } while ({{{ makeGetValue('psrc', 'i-1', 'i8') }}} != 0);
    return pdest;
  },

  strncat__deps: ['strlen'],
  strncat: function(pdest, psrc, num) {
    var len = _strlen(pdest);
    var i = 0;
    while(1) {
      {{{ makeCopyValues('pdest+len+i', 'psrc+i', 1, 'i8', null, 1) }}};
      if ({{{ makeGetValue('pdest', 'len+i', 'i8') }}} == 0) break;
      i ++;
      if (i == num) {
        {{{ makeSetValue('pdest', 'len+i', 0, 'i8') }}}
        break;
      }
    }
    return pdest;
  },

  strcmp__deps: ['strncmp'],
  strcmp: function(px, py) {
    return _strncmp(px, py, TOTAL_MEMORY);
  },
  // We always assume ASCII locale.
  strcoll: 'strcmp',

  strcasecmp__deps: ['strncasecmp'],
  strcasecmp: function(px, py) {
    return _strncasecmp(px, py, TOTAL_MEMORY);
  },

  strncmp: function(px, py, n) {
    var i = 0;
    while (i < n) {
      var x = {{{ makeGetValue('px', 'i', 'i8', 0, 1) }}};
      var y = {{{ makeGetValue('py', 'i', 'i8', 0, 1) }}};
      if (x == y && x == 0) return 0;
      if (x == 0) return -1;
      if (y == 0) return 1;
      if (x == y) {
        i ++;
        continue;
      } else {
        return x > y ? 1 : -1;
      }
    }
    return 0;
  },

  strncasecmp__deps: ['tolower'],
  strncasecmp: function(px, py, n) {
    var i = 0;
    while (i < n) {
      var x = _tolower({{{ makeGetValue('px', 'i', 'i8', 0, 1) }}});
      var y = _tolower({{{ makeGetValue('py', 'i', 'i8', 0, 1) }}});
      if (x == y && x == 0) return 0;
      if (x == 0) return -1;
      if (y == 0) return 1;
      if (x == y) {
        i ++;
        continue;
      } else {
        return x > y ? 1 : -1;
      }
    }
    return 0;
  },

  memcmp: function(p1, p2, num) {
    for (var i = 0; i < num; i++) {
      var v1 = {{{ makeGetValue('p1', 'i', 'i8', 0, 1) }}};
      var v2 = {{{ makeGetValue('p2', 'i', 'i8', 0, 1) }}};
      if (v1 != v2) return v1 > v2 ? 1 : -1;
    }
    return 0;
  },

  memchr: function(ptr, chr, num) {
    chr = unSign(chr);
    for (var i = 0; i < num; i++) {
      if ({{{ makeGetValue('ptr', 0, 'i8') }}} == chr) return ptr;
      ptr++;
    }
    return 0;
  },

  strstr: function(ptr1, ptr2) {
    var check = 0, start;
    do {
      if (!check) {
        start = ptr1;
        check = ptr2;
      }
      var curr1 = {{{ makeGetValue('ptr1++', 0, 'i8') }}};
      var curr2 = {{{ makeGetValue('check++', 0, 'i8') }}};
      if (curr2 == 0) return start;
      if (curr2 != curr1) {
        // rewind to one character after start, to find ez in eeez
        ptr1 = start + 1;
        check = 0;
      }
    } while (curr1);
    return 0;
  },

  strchr: function(ptr, chr) {
    ptr--;
    do {
      ptr++;
      var val = {{{ makeGetValue('ptr', 0, 'i8') }}};
      if (val == chr) return ptr;
    } while (val);
    return 0;
  },

  strrchr__deps: ['strlen'],
  strrchr: function(ptr, chr) {
    var ptr2 = ptr + _strlen(ptr);
    do {
      if ({{{ makeGetValue('ptr2', 0, 'i8') }}} == chr) return ptr2;
      ptr2--;
    } while (ptr2 >= ptr);
    return 0;
  },

  strdup: function(ptr) {
    var len = String_len(ptr);
    var newStr = _malloc(len + 1);
    {{{ makeCopyValues('newStr', 'ptr', 'len', 'null', null, 1) }}};
    {{{ makeSetValue('newStr', 'len', '0', 'i8') }}};
    return newStr;
  },

  strndup__deps: ['strdup'],
  strndup: function(ptr, size) {
    var len = String_len(ptr);

    if (size >= len) {
      return _strdup(ptr);
    }

    if (size < 0) {
      size = 0;
    }
    
    var newStr = _malloc(size + 1);
    {{{ makeCopyValues('newStr', 'ptr', 'size', 'null', null, 1) }}};
    {{{ makeSetValue('newStr', 'size', '0', 'i8') }}};
    return newStr;
  },

  strpbrk: function(ptr1, ptr2) {
    var curr;
    var searchSet = {};
    while (1) {
      var curr = {{{ makeGetValue('ptr2++', 0, 'i8') }}};
      if (!curr) break;
      searchSet[curr] = 1;
    }
    while (1) {
      curr = {{{ makeGetValue('ptr1', 0, 'i8') }}};
      if (!curr) break;
      if (curr in searchSet) return ptr1;
      ptr1++;
    }
    return 0;
  },

  __strtok_state: 0,
  strtok__deps: ['__strtok_state', 'strtok_r'],
  strtok: function(s, delim) {
    if (!___strtok_state) {
      ___strtok_state = _malloc(4);
    }
    return _strtok_r(s, delim, ___strtok_state);
  },

  // Translated from newlib; for the original source and licensing, see library_strtok_r.c
  strtok_r: function(s, delim, lasts) {
    var skip_leading_delim = 1;
    var spanp;
    var c, sc;
    var tok;


    if (s == 0 && (s = getValue(lasts, 'i8*')) == 0) {
      return 0;
    }

    cont: while (1) {
      c = getValue(s++, 'i8');
      for (spanp = delim; (sc = getValue(spanp++, 'i8')) != 0;) {
        if (c == sc) {
          if (skip_leading_delim) {
            continue cont;
          } else {
            setValue(lasts, s, 'i8*');
            setValue(s - 1, 0, 'i8');
            return s - 1;
          }
        }
      }
      break;
    }

    if (c == 0) {
      setValue(lasts, 0, 'i8*');
      return 0;
    }
    tok = s - 1;

    for (;;) {
      c = getValue(s++, 'i8');
      spanp = delim;
      do {
        if ((sc = getValue(spanp++, 'i8')) == c) {
          if (c == 0) {
            s = 0;
          } else {
            setValue(s - 1, 0, 'i8');
          }
          setValue(lasts, s, 'i8*');
          return tok;
        }
      } while (sc != 0);
    }
    abort('strtok_r error!');
  },

  strerror_r__deps: ['$ERRNO_CODES', '$ERRNO_MESSAGES', '__setErrNo'],
  strerror_r: function(errnum, strerrbuf, buflen) {
    if (errnum in ERRNO_MESSAGES) {
      if (ERRNO_MESSAGES[errnum].length > buflen - 1) {
        return ___setErrNo(ERRNO_CODES.ERANGE);
      } else {
        var msg = ERRNO_MESSAGES[errnum];
        for (var i = 0; i < msg.length; i++) {
          {{{ makeSetValue('strerrbuf', 'i', 'msg.charCodeAt(i)', 'i8') }}}
        }
        {{{ makeSetValue('strerrbuf', 'i', 0, 'i8') }}}
        return 0;
      }
    } else {
      return ___setErrNo(ERRNO_CODES.EINVAL);
    }
  },
  strerror__deps: ['strerror_r'],
  strerror: function(errnum) {
    if (!_strerror.buffer) _strerror.buffer = _malloc(256);
    _strerror_r(errnum, _strerror.buffer, 256);
    return _strerror.buffer;
  },

  // ==========================================================================
  // ctype.h
  // ==========================================================================

  isascii: function(chr) {
    return chr >= 0 && (chr & 0x80) == 0;
  },
  toascii: function(chr) {
    return chr & 0x7F;
  },
  toupper: function(chr) {
    if (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) {
      return chr - 'a'.charCodeAt(0) + 'A'.charCodeAt(0);
    } else {
      return chr;
    }
  },
  _toupper: 'toupper',
  tolower: function(chr) {
    if (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0)) {
      return chr - 'A'.charCodeAt(0) + 'a'.charCodeAt(0);
    } else {
      return chr;
    }
  },
  _tolower: 'tolower',
  // The following functions are defined as macros in glibc.
  islower: function(chr) {
    return chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0);
  },
  isupper: function(chr) {
    return chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0);
  },
  isalpha: function(chr) {
    return (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0));
  },
  isdigit: function(chr) {
    return chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0);
  },
  isdigit_l: 'isdigit', // no locale support yet
  isxdigit: function(chr) {
    return (chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0)) ||
           (chr >= 'a'.charCodeAt(0) && chr <= 'f'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'F'.charCodeAt(0));
  },
  isxdigit_l: 'isxdigit', // no locale support yet
  isalnum: function(chr) {
    return (chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0)) ||
           (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0));
  },
  ispunct: function(chr) {
    return (chr >= '!'.charCodeAt(0) && chr <= '/'.charCodeAt(0)) ||
           (chr >= ':'.charCodeAt(0) && chr <= '@'.charCodeAt(0)) ||
           (chr >= '['.charCodeAt(0) && chr <= '`'.charCodeAt(0)) ||
           (chr >= '{'.charCodeAt(0) && chr <= '~'.charCodeAt(0));
  },
  isspace: function(chr) {
    return chr in { 32: 0, 9: 0, 10: 0, 11: 0, 12: 0, 13: 0 };
  },
  isblank: function(chr) {
    return chr == ' '.charCodeAt(0) || chr == '\t'.charCodeAt(0);
  },
  iscntrl: function(chr) {
    return (0 <= chr && chr <= 0x1F) || chr === 0x7F;
  },
  isprint: function(chr) {
    return 0x1F < chr && chr < 0x7F;
  },
  isgraph: 'isprint',
  // Lookup tables for glibc ctype implementation.
  __ctype_b_loc: function() {
    // http://refspecs.freestandards.org/LSB_3.0.0/LSB-Core-generic/LSB-Core-generic/baselib---ctype-b-loc.html
    var me = ___ctype_b_loc;
    if (!me.ret) {
      var values = [
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,8195,8194,8194,8194,8194,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,24577,49156,49156,49156,
        49156,49156,49156,49156,49156,49156,49156,49156,49156,49156,49156,49156,55304,55304,55304,55304,55304,55304,55304,55304,
        55304,55304,49156,49156,49156,49156,49156,49156,49156,54536,54536,54536,54536,54536,54536,50440,50440,50440,50440,50440,
        50440,50440,50440,50440,50440,50440,50440,50440,50440,50440,50440,50440,50440,50440,50440,49156,49156,49156,49156,49156,
        49156,54792,54792,54792,54792,54792,54792,50696,50696,50696,50696,50696,50696,50696,50696,50696,50696,50696,50696,50696,
        50696,50696,50696,50696,50696,50696,50696,49156,49156,49156,49156,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
      ];
      var i16size = {{{ Runtime.getNativeTypeSize('i16') }}};
      var arr = _malloc(values.length * i16size);
      for (var i = 0; i < values.length; i++) {
        {{{ makeSetValue('arr', 'i * i16size', 'values[i]', 'i16') }}}
      }
      me.ret = allocate([arr + 128 * i16size], 'i16*', ALLOC_NORMAL);
    }
    return me.ret;
  },
  __ctype_tolower_loc: function() {
    // http://refspecs.freestandards.org/LSB_3.1.1/LSB-Core-generic/LSB-Core-generic/libutil---ctype-tolower-loc.html
    var me = ___ctype_tolower_loc;
    if (!me.ret) {
      var values = [
        128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,
        158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,
        188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,
        218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,
        248,249,250,251,252,253,254,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
        33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,97,98,99,100,101,102,103,
        104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,91,92,93,94,95,96,97,98,99,100,101,102,103,
        104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,128,129,130,131,132,133,
        134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,
        164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,
        194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
        224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,
        254,255
      ];
      var i32size = {{{ Runtime.getNativeTypeSize('i32') }}};
      var arr = _malloc(values.length * i32size);
      for (var i = 0; i < values.length; i++) {
        {{{ makeSetValue('arr', 'i * i32size', 'values[i]', 'i32') }}}
      }
      me.ret = allocate([arr + 128 * i32size], 'i32*', ALLOC_NORMAL);
    }
    return me.ret;
  },
  __ctype_toupper_loc: function() {
    // http://refspecs.freestandards.org/LSB_3.1.1/LSB-Core-generic/LSB-Core-generic/libutil---ctype-toupper-loc.html
    var me = ___ctype_toupper_loc;
    if (!me.ret) {
      var values = [
        128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,
        158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,
        188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,
        218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,
        248,249,250,251,252,253,254,-1,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,
        33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,
        73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,
        81,82,83,84,85,86,87,88,89,90,123,124,125,126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,
        145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,
        175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,192,193,194,195,196,197,198,199,200,201,202,203,204,
        205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,225,226,227,228,229,230,231,232,233,234,
        235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
      ];
      var i32size = {{{ Runtime.getNativeTypeSize('i32') }}};
      var arr = _malloc(values.length * i32size);
      for (var i = 0; i < values.length; i++) {
        {{{ makeSetValue('arr', 'i * i32size', 'values[i]', 'i32') }}}
      }
      me.ret = allocate([arr + 128 * i32size], 'i32*', ALLOC_NORMAL);
    }
    return me.ret;
  },

  // ==========================================================================
  // LLVM specifics
  // ==========================================================================

  llvm_va_start__inline: function(ptr) {
    // varargs - we received a pointer to the varargs as a final 'extra' parameter
    var data = 'arguments[' + Framework.currItem.funcData.ident + '.length]';
    return makeSetValue(ptr, 0, data, 'void*');
  },

  llvm_va_end: function() {},

  llvm_va_copy: function(ppdest, ppsrc) {
    {{{ makeCopyValues('ppdest', 'ppsrc', Runtime.QUANTUM_SIZE, 'null', null, 1) }}};
    /* Alternate implementation that copies the actual DATA; it assumes the va_list is prefixed by its size
    var psrc = IHEAP[ppsrc]-1;
    var num = IHEAP[psrc]; // right before the data, is the number of (flattened) values
    var pdest = _malloc(num+1);
    _memcpy(pdest, psrc, num+1);
    IHEAP[ppdest] = pdest+1;
    */
  },

  llvm_bswap_i16: function(x) {
    return ((x&0xff)<<8) | ((x>>8)&0xff);
  },

  llvm_bswap_i32: function(x) {
    return ((x&0xff)<<24) | (((x>>8)&0xff)<<16) | (((x>>16)&0xff)<<8) | (x>>>24);
  },

  llvm_bswap_i64__deps: ['llvm_bswap_i32'],
  llvm_bswap_i64: function(l, h) {
    var retl = _llvm_bswap_i32(h)>>>0;
    var reth = _llvm_bswap_i32(l)>>>0;
#if USE_TYPED_ARRAYS == 2
    return [retl, reth];
#else
    throw 'unsupported';
#endif
  },

  llvm_ctlz_i32: function(x) {
    for (var i=0; i<32; i++) {
        if ( (x & (1 << (31-i))) != 0 ) {
            return i;
        }
    }
    return 32;
  },

  llvm_ctlz_i64__deps: ['llvm_ctlz_i32'],
  llvm_ctlz_i64: function(l, h) {
    var ret = _llvm_ctlz_i32(h);
    if (ret == 32) ret += _llvm_ctlz_i32(l);
#if USE_TYPED_ARRAYS == 2
    return [ret, 0];
#else
    return ret;
#endif
  },

  llvm_trap: function() {
    throw 'trap! ' + new Error().stack;
  },

  __assert_fail: function(condition, file, line) {
    ABORT = true;
    throw 'Assertion failed: ' + Pointer_stringify(condition) + ' at ' + new Error().stack;
  },

  __assert_func: function(filename, line, func, condition) {
    throw 'Assertion failed: ' + (condition ? Pointer_stringify(condition) : 'unknown condition') + ', at: ' + [filename ? Pointer_stringify(filename) : 'unknown filename', line, func ? Pointer_stringify(func) : 'unknown function'] + ' at ' + new Error().stack;
  },

  __cxa_guard_acquire: function(variable) {
    if (!{{{ makeGetValue(0, 'variable', 'i8', null, null, 1) }}}) { // ignore SAFE_HEAP stuff because llvm mixes i64 and i8 here
      {{{ makeSetValue(0, 'variable', '1', 'i8') }}};
      return 1;
    }
    return 0;
  },
  __cxa_guard_release: function() {},
  __cxa_guard_abort: function() {},

  _ZTVN10__cxxabiv119__pointer_type_infoE: [0], // is a pointer
  _ZTVN10__cxxabiv117__class_type_infoE: [1], // no inherited classes
  _ZTVN10__cxxabiv120__si_class_type_infoE: [2], // yes inherited classes

  // Exceptions
  __cxa_allocate_exception: function(size) {
    return _malloc(size);
  },
  __cxa_free_exception: function(ptr) {
    return _free(ptr);
  },
  __cxa_throw__deps: ['llvm_eh_exception', '_ZSt18uncaught_exceptionv', '__cxa_find_matching_catch'],
  __cxa_throw: function(ptr, type, destructor) {
    if (!___cxa_throw.initialized) {
      try {
        {{{ makeSetValue('__ZTVN10__cxxabiv119__pointer_type_infoE', '0', '0', 'i32') }}}; // Workaround for libcxxabi integration bug
      } catch(e){}
      try {
        {{{ makeSetValue('__ZTVN10__cxxabiv117__class_type_infoE', '0', '1', 'i32') }}}; // Workaround for libcxxabi integration bug
      } catch(e){}
      try {
        {{{ makeSetValue('__ZTVN10__cxxabiv120__si_class_type_infoE', '0', '2', 'i32') }}}; // Workaround for libcxxabi integration bug
      } catch(e){}
      ___cxa_throw.initialized = true;
    }
#if EXCEPTION_DEBUG
    Module.printErr('Compiled code throwing an exception, ' + [ptr,type,destructor] + ', at ' + new Error().stack);
#endif
    {{{ makeSetValue('_llvm_eh_exception.buf', '0', 'ptr', 'void*') }}}
    {{{ makeSetValue('_llvm_eh_exception.buf', QUANTUM_SIZE, 'type', 'void*') }}}
    {{{ makeSetValue('_llvm_eh_exception.buf', 2 * QUANTUM_SIZE, 'destructor', 'void*') }}}
    if (!("uncaught_exception" in __ZSt18uncaught_exceptionv)) {
      __ZSt18uncaught_exceptionv.uncaught_exception = 1;
    } else {
      __ZSt18uncaught_exceptionv.uncaught_exception++;
    }
    throw ptr;
  },
  __cxa_rethrow__deps: ['llvm_eh_exception', '__cxa_end_catch'],
  __cxa_rethrow: function() {
    ___cxa_end_catch.rethrown = true;
    throw {{{ makeGetValue('_llvm_eh_exception.buf', '0', 'void*') }}};
  },
  llvm_eh_exception__postset: '_llvm_eh_exception.buf = allocate(12, "void*", ALLOC_STATIC);',
  llvm_eh_exception: function() {
    return {{{ makeGetValue('_llvm_eh_exception.buf', '0', 'void*') }}};
  },
  llvm_eh_selector__jsargs: true,
  llvm_eh_selector: function(unused_exception_value, personality/*, varargs*/) {
    var type = {{{ makeGetValue('_llvm_eh_exception.buf', QUANTUM_SIZE, 'void*') }}}
    for (var i = 2; i < arguments.length; i++) {
      if (arguments[i] ==  type) return type;
    }
    return 0;
  },
  llvm_eh_typeid_for: function(type) {
    return type;
  },
  __cxa_begin_catch__deps: ['_ZSt18uncaught_exceptionv'],
  __cxa_begin_catch: function(ptr) {
    __ZSt18uncaught_exceptionv.uncaught_exception--;
    return ptr;
  },
  __cxa_end_catch__deps: ['llvm_eh_exception', '__cxa_free_exception'],
  __cxa_end_catch: function() {
    if (___cxa_end_catch.rethrown) {
      ___cxa_end_catch.rethrown = false;
      return;
    }
    // Clear state flag.
    __THREW__ = false;
    // Clear type.
    {{{ makeSetValue('_llvm_eh_exception.buf', QUANTUM_SIZE, '0', 'void*') }}}
    // Call destructor if one is registered then clear it.
    var ptr = {{{ makeGetValue('_llvm_eh_exception.buf', '0', 'void*') }}};
    var destructor = {{{ makeGetValue('_llvm_eh_exception.buf', 2 * QUANTUM_SIZE, 'void*') }}};
    if (destructor) {
      FUNCTION_TABLE[destructor](ptr);
      {{{ makeSetValue('_llvm_eh_exception.buf', 2 * QUANTUM_SIZE, '0', 'i32') }}}
    }
    // Free ptr if it isn't null.
    if (ptr) {
      ___cxa_free_exception(ptr);
      {{{ makeSetValue('_llvm_eh_exception.buf', '0', '0', 'void*') }}}
    }
  },
  __cxa_get_exception_ptr__deps: ['llvm_eh_exception'],
  __cxa_get_exception_ptr: function(ptr) {
    return ptr;
  },
  _ZSt18uncaught_exceptionv: function() { // std::uncaught_exception()
    return !!__ZSt18uncaught_exceptionv.uncaught_exception;
  },

  __cxa_call_unexpected: function(exception) {
    ABORT = true;
    throw exception;
  },

  _Unwind_Resume_or_Rethrow: function(ptr) {
    throw ptr;
  },
  _Unwind_RaiseException__deps: ['llvm_eh_exception', '__cxa_find_matching_catch'],
  _Unwind_RaiseException: function(ptr) {
    throw ptr;
  },
  _Unwind_DeleteException: function(ptr) {},

  terminate: '__cxa_call_unexpected',

  __gxx_personality_v0: function() {
  },

  __cxa_is_number_type: function(type) {
    var isNumber = false;
    try { if (type == __ZTIi) isNumber = true } catch(e){}
    try { if (type == __ZTIj) isNumber = true } catch(e){}
    try { if (type == __ZTIl) isNumber = true } catch(e){}
    try { if (type == __ZTIm) isNumber = true } catch(e){}
    try { if (type == __ZTIx) isNumber = true } catch(e){}
    try { if (type == __ZTIy) isNumber = true } catch(e){}
    try { if (type == __ZTIf) isNumber = true } catch(e){}
    try { if (type == __ZTId) isNumber = true } catch(e){}
    try { if (type == __ZTIe) isNumber = true } catch(e){}
    try { if (type == __ZTIc) isNumber = true } catch(e){}
    try { if (type == __ZTIa) isNumber = true } catch(e){}
    try { if (type == __ZTIh) isNumber = true } catch(e){}
    try { if (type == __ZTIs) isNumber = true } catch(e){}
    try { if (type == __ZTIt) isNumber = true } catch(e){}
    return isNumber;
  },

  // Finds a suitable catch clause for when an exception is thrown.
  // In normal compilers, this functionality is handled by the C++
  // 'personality' routine. This is passed a fairly complex structure
  // relating to the context of the exception and makes judgements
  // about how to handle it. Some of it is about matching a suitable
  // catch clause, and some of it is about unwinding. We already handle
  // unwinding using 'if' blocks around each function, so the remaining
  // functionality boils down to picking a suitable 'catch' block.
  // We'll do that here, instead, to keep things simpler.

  __cxa_find_matching_catch__deps: ['__cxa_does_inherit', '__cxa_is_number_type'],
  __cxa_find_matching_catch: function(thrown, throwntype, typeArray) {
    // If throwntype is a pointer, this means a pointer has been
    // thrown. When a pointer is thrown, actually what's thrown
    // is a pointer to the pointer. We'll dereference it.
    if (throwntype != 0 && !___cxa_is_number_type(throwntype)) {
      var throwntypeInfoAddr= {{{ makeGetValue('throwntype', '0', '*') }}} - {{{ Runtime.QUANTUM_SIZE*2 }}};
      var throwntypeInfo= {{{ makeGetValue('throwntypeInfoAddr', '0', '*') }}};
      if (throwntypeInfo == 0)
        thrown = {{{ makeGetValue('thrown', '0', '*') }}};
    }
    // The different catch blocks are denoted by different types.
    // Due to inheritance, those types may not precisely match the
    // type of the thrown object. Find one which matches, and
    // return the type of the catch block which should be called.
    for (var i = 0; i < typeArray.length; i++) {
      if (___cxa_does_inherit(typeArray[i], throwntype, thrown))
        return { f0:thrown, f1:typeArray[i] };
    }
    // Shouldn't happen unless we have bogus data in typeArray
    // or encounter a type for which emscripten doesn't have suitable
    // typeinfo defined. Best-efforts match just in case.
    return { f0:thrown, f1 :throwntype };
  },

  // Recursively walks up the base types of 'possibilityType'
  // to see if any of them match 'definiteType'.
  __cxa_does_inherit__deps: ['__cxa_is_number_type'],
  __cxa_does_inherit: function(definiteType, possibilityType, possibility) {
    if (possibility == 0) return false;
    if (possibilityType == 0 || possibilityType == definiteType)
      return true;
    var possibility_type_info;
    if (___cxa_is_number_type(possibilityType)) {
      possibility_type_info = possibilityType;
    } else {
      var possibility_type_infoAddr = {{{ makeGetValue('possibilityType', '0', '*') }}} - {{{ Runtime.QUANTUM_SIZE*2 }}};
      possibility_type_info = {{{ makeGetValue('possibility_type_infoAddr', '0', '*') }}};
    }
    switch (possibility_type_info) {
    case 0: // possibility is a pointer
      // See if definite type is a pointer
      var definite_type_infoAddr = {{{ makeGetValue('definiteType', '0', '*') }}} - {{{ Runtime.QUANTUM_SIZE*2 }}};
      var definite_type_info = {{{ makeGetValue('definite_type_infoAddr', '0', '*') }}};
      if (definite_type_info == 0) {
        // Also a pointer; compare base types of pointers
        var defPointerBaseAddr = definiteType+{{{ Runtime.QUANTUM_SIZE*2 }}};
        var defPointerBaseType = {{{ makeGetValue('defPointerBaseAddr', '0', '*') }}};
        var possPointerBaseAddr = possibilityType+{{{ Runtime.QUANTUM_SIZE*2 }}};
        var possPointerBaseType = {{{ makeGetValue('possPointerBaseAddr', '0', '*') }}};
        return ___cxa_does_inherit(defPointerBaseType, possPointerBaseType, possibility);
      } else
        return false; // one pointer and one non-pointer
    case 1: // class with no base class
      return false;
    case 2: // class with base class
      var parentTypeAddr = possibilityType + {{{ Runtime.QUANTUM_SIZE*2 }}};
      var parentType = {{{ makeGetValue('parentTypeAddr', '0', '*') }}};
      return ___cxa_does_inherit(definiteType, parentType, possibility);
    default:
      return false; // some unencountered type
    }
  },

  // RTTI hacks for exception handling, defining type_infos for common types.
  // The values are dummies. We simply use the addresses of these statically
  // allocated variables as unique identifiers.
  _ZTIb: [0], // bool
  _ZTIi: [0], // int
  _ZTIj: [0], // unsigned int
  _ZTIl: [0], // long
  _ZTIm: [0], // unsigned long
  _ZTIx: [0], // long long
  _ZTIy: [0], // unsigned long long
  _ZTIf: [0], // float
  _ZTId: [0], // double
  _ZTIe: [0], // long double
  _ZTIc: [0], // char
  _ZTIa: [0], // signed char
  _ZTIh: [0], // unsigned char
  _ZTIs: [0], // short
  _ZTIt: [0], // unsigned short
  _ZTIv: [0], // void
  _ZTIPv: [0], // void*

  llvm_uadd_with_overflow_i8: function(x, y) {
    x = x & 0xff;
    y = y & 0xff;
    return {
      f0: (x+y) & 0xff,
      f1: x+y > 255
    };
  },

  llvm_umul_with_overflow_i8: function(x, y) {
    x = x & 0xff;
    y = y & 0xff;
    return {
      f0: (x*y) & 0xff,
      f1: x*y > 255
    };
  },

  llvm_uadd_with_overflow_i16: function(x, y) {
    x = x & 0xffff;
    y = y & 0xffff;
    return {
      f0: (x+y) & 0xffff,
      f1: x+y > 65535
    };
  },

  llvm_umul_with_overflow_i16: function(x, y) {
    x = x & 0xffff;
    y = y & 0xffff;
    return {
      f0: (x*y) & 0xffff,
      f1: x*y > 65535
    };
  },

  llvm_uadd_with_overflow_i32: function(x, y) {
    x = x>>>0;
    y = y>>>0;
    return {
      f0: (x+y)>>>0,
      f1: x+y > 4294967295
    };
  },

  llvm_umul_with_overflow_i32: function(x, y) {
    x = x>>>0;
    y = y>>>0;
    return {
      f0: (x*y)>>>0,
      f1: x*y > 4294967295
    };
  },

  llvm_uadd_with_overflow_i64__deps: [function() { Types.preciseI64MathUsed = 1 }],
  llvm_uadd_with_overflow_i64: function(xl, xh, yl, yh) {
    i64Math.add(xl, xh, yl, yh);
    return {
      f0: i64Math.result,
      f1: 0 // XXX Need to hack support for this in long.js
    };
  },

  llvm_umul_with_overflow_i64__deps: [function() { Types.preciseI64MathUsed = 1 }],
  llvm_umul_with_overflow_i64: function(xl, xh, yl, yh) {
    i64Math.mul(xl, xh, yl, yh);
    return {
      f0: i64Math.result,
      f1: 0 // XXX Need to hack support for this in long.js
    };
  },

  llvm_stacksave: function() {
    var self = _llvm_stacksave;
    if (!self.LLVM_SAVEDSTACKS) {
      self.LLVM_SAVEDSTACKS = [];
    }
    self.LLVM_SAVEDSTACKS.push(Runtime.stackSave());
    return self.LLVM_SAVEDSTACKS.length-1;
  },
  llvm_stackrestore: function(p) {
    var self = _llvm_stacksave;
    var ret = self.LLVM_SAVEDSTACKS[p];
    self.LLVM_SAVEDSTACKS.splice(p, 1);
    Runtime.stackRestore(ret);
  },

  __cxa_pure_virtual: function() {
    ABORT = true;
    throw 'Pure virtual function called!';
  },

  llvm_flt_rounds: function() {
    return -1; // 'indeterminable' for FLT_ROUNDS
  },

  llvm_memory_barrier: function(){},

  llvm_atomic_load_add_i32_p0i32: function(ptr, delta) {
    var ret = {{{ makeGetValue('ptr', '0', 'i32') }}};
    {{{ makeSetValue('ptr', '0', 'ret+delta', 'i32') }}};
    return ret;
  },

  llvm_expect_i32__inline: function(val, expected) {
    return '(' + val + ')';
  },

  llvm_lifetime_start: function() {},
  llvm_lifetime_end: function() {},

  llvm_invariant_start: function() {},
  llvm_invariant_end: function() {},

  llvm_objectsize_i32: function() { return -1 }, // TODO: support this

  // ==========================================================================
  // llvm-mono integration
  // ==========================================================================

  llvm_mono_load_i8_p0i8: function(ptr) {
    return {{{ makeGetValue('ptr', 0, 'i8') }}};
  },

  llvm_mono_store_i8_p0i8: function(value, ptr) {
    {{{ makeSetValue('ptr', 0, 'value', 'i8') }}};
  },

  llvm_mono_load_i16_p0i16: function(ptr) {
    return {{{ makeGetValue('ptr', 0, 'i16') }}};
  },

  llvm_mono_store_i16_p0i16: function(value, ptr) {
    {{{ makeSetValue('ptr', 0, 'value', 'i16') }}};
  },

  llvm_mono_load_i32_p0i32: function(ptr) {
    return {{{ makeGetValue('ptr', 0, 'i32') }}};
  },

  llvm_mono_store_i32_p0i32: function(value, ptr) {
    {{{ makeSetValue('ptr', 0, 'value', 'i32') }}};
  },

  // ==========================================================================
  // math.h
  // ==========================================================================

  cos: 'Math.cos',
  cosf: 'Math.cos',
  sin: 'Math.sin',
  sinf: 'Math.sin',
  tan: 'Math.tan',
  tanf: 'Math.tan',
  acos: 'Math.acos',
  acosf: 'Math.acos',
  asin: 'Math.asin',
  asinf: 'Math.asin',
  atan: 'Math.atan',
  atanf: 'Math.atan',
  atan2: 'Math.atan2',
  atan2f: 'Math.atan2',
  exp: 'Math.exp',
  expf: 'Math.exp',

  // The erf and erfc functions are inspired from
  // http://www.digitalmars.com/archives/cplusplus/3634.html
  // and mruby source code at
  // https://github.com/mruby/mruby/blob/master/src/math.c
  erfc: function (x) {
    var MATH_TOLERANCE = 1E-12;
    var ONE_SQRTPI = 0.564189583547756287;
    var a = 1;
    var b = x;
    var c = x;
    var d = x * x + 0.5;
    var n = 1.0;
    var q2 = b / d;
    var q1, t;

    if (Math.abs(x) < 2.2) {
      return 1.0 - _erf(x);
    }
    if (x < 0) {
      return 2.0 - _erfc(-x);
    }
    do {
      t = a * n + b * x;
      a = b;
      b = t;
      t = c * n + d * x;
      c = d;
      d = t;
      n += 0.5;
      q1 = q2;
      q2 = b / d;
    } while (Math.abs(q1 - q2) / q2 > MATH_TOLERANCE);
    return (ONE_SQRTPI * Math.exp(- x * x) * q2);
  },
  erfcf: 'erfcf',
  erf: function (x) {
    var MATH_TOLERANCE = 1E-12;
    var TWO_SQRTPI = 1.128379167095512574;
    var sum = x;
    var term = x;
    var xsqr = x*x;
    var j = 1;

    if (Math.abs(x) > 2.2) {
      return 1.0 - _erfc(x);
    }
    do {
      term *= xsqr / j;
      sum -= term / (2 * j + 1);
      ++j;
      term *= xsqr / j;
      sum += term / (2 * j + 1);
      ++j;
    } while (Math.abs(term / sum) > MATH_TOLERANCE);
    return (TWO_SQRTPI * sum);
  },
  erff: 'erf',
  log: 'Math.log',
  logf: 'Math.log',
  sqrt: 'Math.sqrt',
  sqrtf: 'Math.sqrt',
  fabs: 'Math.abs',
  fabsf: 'Math.abs',
  ceil: 'Math.ceil',
  ceilf: 'Math.ceil',
  floor: 'Math.floor',
  floorf: 'Math.floor',
  pow: 'Math.pow',
  powf: 'Math.pow',
  llvm_sqrt_f32: 'Math.sqrt',
  llvm_sqrt_f64: 'Math.sqrt',
  llvm_pow_f32: 'Math.pow',
  llvm_pow_f64: 'Math.pow',
  llvm_log_f32: 'Math.log',
  llvm_log_f64: 'Math.log',
  llvm_exp_f32: 'Math.exp',
  llvm_exp_f64: 'Math.exp',
  ldexp: function(x, exp_) {
    return x * Math.pow(2, exp_);
  },
  ldexpf: 'ldexp',
  scalb: 'ldexp',
  scalbn: 'ldexp',
  scalbnf: 'ldexp',
  scalbln: 'ldexp',
  scalblnf: 'ldexp',
  cbrt: function(x) {
    return Math.pow(x, 1/3);
  },
  cbrtf: 'cbrt',
  cbrtl: 'cbrt',

  modf: function(x, intpart) {
    {{{ makeSetValue('intpart', 0, 'Math.floor(x)', 'double') }}}
    return x - {{{ makeGetValue('intpart', 0, 'double') }}};
  },
  modff: function(x, intpart) {
    {{{ makeSetValue('intpart', 0, 'Math.floor(x)', 'float') }}}
    return x - {{{ makeGetValue('intpart', 0, 'float') }}};
  },
  frexp: function(x, exp_addr) {
    var sig = 0, exp_ = 0;
    if (x !== 0) {
      var raw_exp = Math.log(x)/Math.log(2);
      exp_ = Math.ceil(raw_exp);
      if (exp_ === raw_exp) exp_ += 1;
      sig = x/Math.pow(2, exp_);
    }
    {{{ makeSetValue('exp_addr', 0, 'exp_', 'i32') }}}
    return sig;
  },
  frexpf: 'frexp',
  finite: function(x) {
    return isFinite(x);
  },
  __finite: 'finite',
  isinf: function(x) {
    return !isNaN(x) && !isFinite(x);
  },
  __isinf: 'isinf',
  isnan: function(x) {
    return isNaN(x);
  },
  __isnan: 'isnan',
  copysign: function(a, b) {
      if (a < 0 === b < 0) return a;
      return -a;
  },
  copysignf: 'copysign',
  hypot: function(a, b) {
     return Math.sqrt(a*a + b*b);
  },
  hypotf: 'hypot',
  sinh: function(x) {
    var p = Math.pow(Math.E, x);
    return (p - (1 / p)) / 2;
  },
  sinhf: 'sinh',
  cosh: function(x) {
    var p = Math.pow(Math.E, x);
    return (p + (1 / p)) / 2;
  },
  coshf: 'cosh',
  tanh__deps: ['sinh', 'cosh'],
  tanh: function(x) {
    return _sinh(x) / _cosh(x);
  },
  tanhf: 'tanh',
  asinh: function(x) {
    return Math.log(x + Math.sqrt(x * x + 1));
  },
  asinhf: 'asinh',
  acosh: function(x) {
    return Math.log(x * 1 + Math.sqrt(x * x - 1));
  },
  acoshf: 'acosh',
  atanh: function(x) {
    return Math.log((1 + x) / (1 - x)) / 2;
  },
  atanhf: 'atanh',
  exp2: function(x) {
    return Math.pow(2, x);
  },
  exp2f: 'exp2',
  expm1: function(x) {
    return Math.exp(x) - 1;
  },
  expm1f: 'expm1',
  round: function(x) {
    return (x < 0) ? -Math.round(-x) : Math.round(x);
  },
  roundf: 'round',
  lround: 'round',
  lroundf: 'round',
  llround: 'round',
  llroundf: 'round',
  rint: function(x) {
    return (x > 0) ? -Math.round(-x) : Math.round(x);
  },
  rintf: 'rint',
  lrint: 'rint',
  lrintf: 'rint',
  llrint: 'rint',
  llrintf: 'rint',
  nearbyint: 'rint',
  nearbyintf: 'rint',
  trunc: function(x) {
    return (x < 0) ? Math.ceil(x) : Math.floor(x);
  },
  truncf: 'trunc',
  fdim: function(x, y) {
    return (x > y) ? x - y : 0;
  },
  fdimf: 'fdim',
  fmax: function(x, y) {
    return isNaN(x) ? y : isNaN(y) ? x : Math.max(x, y);
  },
  fmaxf: 'fmax',
  fmin: function(x, y) {
    return isNaN(x) ? y : isNaN(y) ? x : Math.min(x, y);
  },
  fminf: 'fmin',
  fma: function(x, y, z) {
    return x * y + z;
  },
  fmaf: 'fma',
  fmod: function(x, y) {
    return x % y;
  },
  fmodf: 'fmod',
  remainder: 'fmod',
  remainderf: 'fmod',
  log10: function(x) {
    return Math.log(x) / Math.LN10;
  },
  log10f: 'log10',
  log1p: function(x) {
    return Math.log(1 + x);
  },
  log1pf: 'log1p',
  log2: function(x) {
    return Math.log(x) / Math.LN2;
  },
  log2f: 'log2',
  nan: function(x) {
    return NaN;
  },
  nanf: 'nan',

  sincos: function(x, sine, cosine) {
    var sineVal = Math.sin(x),
        cosineVal = Math.cos(x);
    {{{ makeSetValue('sine', '0', 'sineVal', 'double') }}};
    {{{ makeSetValue('cosine', '0', 'cosineVal', 'double') }}};
  },

  sincosf: function(x, sine, cosine) {
    var sineVal = Math.sin(x),
        cosineVal = Math.cos(x);
    {{{ makeSetValue('sine', '0', 'sineVal', 'float') }}};
    {{{ makeSetValue('cosine', '0', 'cosineVal', 'float') }}};
  },

  __div_t_struct_layout: Runtime.generateStructInfo([
                            ['i32', 'quot'],
                            ['i32', 'rem'],
                          ]),
  div__deps: ['__div_t_struct_layout'],
  div: function(divt, numer, denom) {
    var quot = Math.floor(numer / denom);
    var rem = numer - quot * denom;
    var offset = ___div_t_struct_layout.rem;
    {{{ makeSetValue('divt', '0', 'quot', 'i32') }}};
    {{{ makeSetValue('divt', 'offset', 'rem', 'i32') }}};
    return divt;
  },

  __fpclassifyf: function(x) {
    if (isNaN(x)) return {{{ cDefine('FP_NAN') }}};
    if (!isFinite(x)) return {{{ cDefine('FP_INFINITE') }}};
    if (x == 0) return {{{ cDefine('FP_ZERO') }}};
    // FP_SUBNORMAL..?
    return {{{ cDefine('FP_NORMAL') }}};
  },
  __fpclassifyd: '__fpclassifyf',

  // ==========================================================================
  // sys/utsname.h
  // ==========================================================================

  __utsname_struct_layout: Runtime.generateStructInfo([
	  'sysname',
	  'nodename',
	  'release',
	  'version',
	  'machine'], '%struct.utsname'),
  uname__deps: ['__utsname_struct_layout'],
  uname: function(name) {
    // int uname(struct utsname *name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/uname.html
    function copyString(element, value) {
      var offset = ___utsname_struct_layout[element];
      for (var i = 0; i < value.length; i++) {
        {{{ makeSetValue('name', 'offset + i', 'value.charCodeAt(i)', 'i8') }}}
      }
      {{{ makeSetValue('name', 'offset + i', '0', 'i8') }}}
    }
    if (name === 0) {
      return -1;
    } else {
      copyString('sysname', 'Emscripten');
      copyString('nodename', 'emscripten');
      copyString('release', '1.0');
      copyString('version', '#1');
      copyString('machine', 'x86-JS');
      return 0;
    }
  },

  // ==========================================================================
  // dlfcn.h - Dynamic library loading
  //
  // Some limitations:
  //
  //  * Minification on each file separately may not work, as they will
  //    have different shortened names. You can in theory combine them, then
  //    minify, then split... perhaps.
  //
  //  * LLVM optimizations may fail. If the child wants to access a function
  //    in the parent, LLVM opts may remove it from the parent when it is
  //    being compiled. Not sure how to tell LLVM to not do so.
  // ==========================================================================

  // Data for dlfcn.h.
  $DLFCN_DATA: {
    error: null,
    errorMsg: null,
    loadedLibs: {}, // handle -> [refcount, name, lib_object]
    loadedLibNames: {}, // name -> handle
  },
  // void* dlopen(const char* filename, int flag);
  dlopen__deps: ['$DLFCN_DATA', '$FS', '$ENV'],
  dlopen: function(filename, flag) {
    // void *dlopen(const char *file, int mode);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlopen.html
    filename = (ENV['LD_LIBRARY_PATH'] || '/') + Pointer_stringify(filename);

    if (DLFCN_DATA.loadedLibNames[filename]) {
      // Already loaded; increment ref count and return.
      var handle = DLFCN_DATA.loadedLibNames[filename];
      DLFCN_DATA.loadedLibs[handle].refcount++;
      return handle;
    }

    var target = FS.findObject(filename);
    if (!target || target.isFolder || target.isDevice) {
      DLFCN_DATA.errorMsg = 'Could not find dynamic lib: ' + filename;
      return 0;
    } else {
      FS.forceLoadFile(target);
      var lib_data = intArrayToString(target.contents);
    }

    try {
      var lib_module = eval(lib_data)(FUNCTION_TABLE.length);
    } catch (e) {
#if ASSERTIONS
      Module.printErr('Error in loading dynamic library: ' + e);
#endif
      DLFCN_DATA.errorMsg = 'Could not evaluate dynamic lib: ' + filename;
      return 0;
    }

    // Not all browsers support Object.keys().
    var handle = 1;
    for (var key in DLFCN_DATA.loadedLibs) {
      if (DLFCN_DATA.loadedLibs.hasOwnProperty(key)) handle++;
    }

    DLFCN_DATA.loadedLibs[handle] = {
      refcount: 1,
      name: filename,
      module: lib_module,
      cached_functions: {}
    };
    DLFCN_DATA.loadedLibNames[filename] = handle;

    // We don't care about RTLD_NOW and RTLD_LAZY.
    if (flag & 256) { // RTLD_GLOBAL
      for (var ident in lib_module) {
        if (lib_module.hasOwnProperty(ident)) {
          Module[ident] = lib_module[ident];
        }
      }
    }

    return handle;
  },
  // int dlclose(void* handle);
  dlclose__deps: ['$DLFCN_DATA'],
  dlclose: function(handle) {
    // int dlclose(void *handle);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlclose.html
    if (!DLFCN_DATA.loadedLibs[handle]) {
      DLFCN_DATA.errorMsg = 'Tried to dlclose() unopened handle: ' + handle;
      return 1;
    } else {
      var lib_record = DLFCN_DATA.loadedLibs[handle];
      if (lib_record.refcount-- == 0) {
        delete DLFCN_DATA.loadedLibNames[lib_record.name];
        delete DLFCN_DATA.loadedLibs[handle];
      }
      return 0;
    }
  },
  // void* dlsym(void* handle, const char* symbol);
  dlsym__deps: ['$DLFCN_DATA'],
  dlsym: function(handle, symbol) {
    // void *dlsym(void *restrict handle, const char *restrict name);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlsym.html
    symbol = '_' + Pointer_stringify(symbol);

    if (!DLFCN_DATA.loadedLibs[handle]) {
      DLFCN_DATA.errorMsg = 'Tried to dlsym() from an unopened handle: ' + handle;
      return 0;
    } else {
      var lib = DLFCN_DATA.loadedLibs[handle];
      if (!lib.module.hasOwnProperty(symbol)) {
        DLFCN_DATA.errorMsg = ('Tried to lookup unknown symbol "' + symbol +
                               '" in dynamic lib: ' + lib.name);
        return 0;
      } else {
        if (lib.cached_functions.hasOwnProperty(symbol)) {
          return lib.cached_functions[symbol];
        } else {
          var result = lib.module[symbol];
          if (typeof result == 'function') {
            FUNCTION_TABLE.push(result);
            FUNCTION_TABLE.push(0);
            result = FUNCTION_TABLE.length - 2;
            lib.cached_functions = result;
          }
          return result;
        }
      }
    }
  },
  // char* dlerror(void);
  dlerror__deps: ['$DLFCN_DATA'],
  dlerror: function() {
    // char *dlerror(void);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/dlerror.html
    if (DLFCN_DATA.errorMsg === null) {
      return 0;
    } else {
      if (DLFCN_DATA.error) _free(DLFCN_DATA.error);
      var msgArr = intArrayFromString(DLFCN_DATA.errorMsg);
      DLFCN_DATA.error = allocate(msgArr, 'i8', ALLOC_NORMAL);
      DLFCN_DATA.errorMsg = null;
      return DLFCN_DATA.error;
    }
  },

  // ==========================================================================
  // pwd.h
  // ==========================================================================

  // TODO: Implement.
  // http://pubs.opengroup.org/onlinepubs/009695399/basedefs/pwd.h.html
  getpwuid: function(uid) {
    return 0; // NULL
  },

  // ==========================================================================
  // time.h
  // ==========================================================================

  clock: function() {
    if (_clock.start === undefined) _clock.start = Date.now();
    return Math.floor((Date.now() - _clock.start) * ({{{ cDefine('CLOCKS_PER_SEC') }}}/1000));
  },

  time: function(ptr) {
    var ret = Math.floor(Date.now()/1000);
    if (ptr) {
      {{{ makeSetValue('ptr', 0, 'ret', 'i32') }}}
    }
    return ret;
  },

  difftime: function(time1, time0) {
    return time1 - time0;
  },

  __tm_struct_layout: Runtime.generateStructInfo([
    'tm_sec',
    'tm_min',
    'tm_hour',
    'tm_mday',
    'tm_mon',
    'tm_year',
    'tm_wday',
    'tm_yday',
    'tm_isdst',
    'tm_gmtoff',
    'tm_zone'], '%struct.tm'),
  // Statically allocated time struct.
  __tm_current: 0,
  // Statically allocated timezone strings.
  __tm_timezones: {},
  // Statically allocated time strings.
  __tm_formatted: 0,

  mktime__deps: ['__tm_struct_layout', 'tzset'],
  mktime: function(tmPtr) {
    _tzset();
    var offsets = ___tm_struct_layout;
    var year = {{{ makeGetValue('tmPtr', 'offsets.tm_year', 'i32') }}};
    var timestamp = new Date(year >= 1900 ? year : year + 1900,
                             {{{ makeGetValue('tmPtr', 'offsets.tm_mon', 'i32') }}},
                             {{{ makeGetValue('tmPtr', 'offsets.tm_mday', 'i32') }}},
                             {{{ makeGetValue('tmPtr', 'offsets.tm_hour', 'i32') }}},
                             {{{ makeGetValue('tmPtr', 'offsets.tm_min', 'i32') }}},
                             {{{ makeGetValue('tmPtr', 'offsets.tm_sec', 'i32') }}},
                             0).getTime() / 1000;
    {{{ makeSetValue('tmPtr', 'offsets.tm_wday', 'new Date(timestamp).getDay()', 'i32') }}}
    var yday = Math.round((timestamp - (new Date(year, 0, 1)).getTime()) / (1000 * 60 * 60 * 24));
    {{{ makeSetValue('tmPtr', 'offsets.tm_yday', 'yday', 'i32') }}}
    return timestamp;
  },
  timelocal: 'mktime',

  gmtime__deps: ['malloc', '__tm_struct_layout', '__tm_current', 'gmtime_r'],
  gmtime: function(time) {
    if (!___tm_current) ___tm_current = _malloc(___tm_struct_layout.__size__);
    return _gmtime_r(time, ___tm_current);
  },

  gmtime_r__deps: ['__tm_struct_layout', '__tm_timezones'],
  gmtime_r: function(time, tmPtr) {
    var date = new Date({{{ makeGetValue('time', 0, 'i32') }}}*1000);
    var offsets = ___tm_struct_layout;
    {{{ makeSetValue('tmPtr', 'offsets.tm_sec', 'date.getUTCSeconds()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_min', 'date.getUTCMinutes()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_hour', 'date.getUTCHours()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_mday', 'date.getUTCDate()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_mon', 'date.getUTCMonth()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_year', 'date.getUTCFullYear()-1900', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_wday', 'date.getUTCDay()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_gmtoff', '0', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_isdst', '0', 'i32') }}}

    var start = new Date(date.getFullYear(), 0, 1);
    var yday = Math.round((date.getTime() - start.getTime()) / (1000 * 60 * 60 * 24));
    {{{ makeSetValue('tmPtr', 'offsets.tm_yday', 'yday', 'i32') }}}

    var timezone = "GMT";
    if (!(timezone in ___tm_timezones)) {
      ___tm_timezones[timezone] = allocate(intArrayFromString(timezone), 'i8', ALLOC_NORMAL);
    }
    {{{ makeSetValue('tmPtr', 'offsets.tm_zone', '___tm_timezones[timezone]', 'i32') }}}

    return tmPtr;
  },

  timegm__deps: ['mktime'],
  timegm: function(tmPtr) {
    _tzset();
    var offset = {{{ makeGetValue('__timezone', 0, 'i32') }}};
    var daylight = {{{ makeGetValue('__daylight', 0, 'i32') }}};
    daylight = (daylight == 1) ? 60 * 60 : 0;
    var ret = _mktime(tmPtr) + offset - daylight;
    return ret;
  },

  localtime__deps: ['malloc', '__tm_struct_layout', '__tm_current', 'localtime_r'],
  localtime: function(time) {
    if (!___tm_current) ___tm_current = _malloc(___tm_struct_layout.__size__);
    return _localtime_r(time, ___tm_current);
  },

  localtime_r__deps: ['__tm_struct_layout', '__tm_timezones', 'tzset'],
  localtime_r: function(time, tmPtr) {
    _tzset();
    var offsets = ___tm_struct_layout;
    var date = new Date({{{ makeGetValue('time', 0, 'i32') }}}*1000);
    {{{ makeSetValue('tmPtr', 'offsets.tm_sec', 'date.getSeconds()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_min', 'date.getMinutes()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_hour', 'date.getHours()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_mday', 'date.getDate()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_mon', 'date.getMonth()', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_year', 'date.getFullYear()-1900', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_wday', 'date.getDay()', 'i32') }}}

    var start = new Date(date.getFullYear(), 0, 1);
    var yday = Math.floor((date.getTime() - start.getTime()) / (1000 * 60 * 60 * 24));
    {{{ makeSetValue('tmPtr', 'offsets.tm_yday', 'yday', 'i32') }}}
    {{{ makeSetValue('tmPtr', 'offsets.tm_gmtoff', 'start.getTimezoneOffset() * 60', 'i32') }}}

    var dst = Number(start.getTimezoneOffset() != date.getTimezoneOffset());
    {{{ makeSetValue('tmPtr', 'offsets.tm_isdst', 'dst', 'i32') }}}

    var timezone = 'GMT'; // XXX do not rely on browser timezone info, it is very unpredictable | date.toString().match(/\(([A-Z]+)\)/)[1];
    if (!(timezone in ___tm_timezones)) {
      ___tm_timezones[timezone] = allocate(intArrayFromString(timezone), 'i8', ALLOC_NORMAL);
    }
    {{{ makeSetValue('tmPtr', 'offsets.tm_zone', '___tm_timezones[timezone]', 'i32') }}}

    return tmPtr;
  },

  asctime__deps: ['malloc', '__tm_formatted', 'asctime_r'],
  asctime: function(tmPtr) {
    if (!___tm_formatted) ___tm_formatted = _malloc(26);
    return _asctime_r(tmPtr, ___tm_formatted);
  },

  asctime_r__deps: ['__tm_formatted', 'mktime'],
  asctime_r: function(tmPtr, buf) {
    var date = new Date(_mktime(tmPtr)*1000);
    var formatted = date.toString();
    var datePart = formatted.replace(/\d{4}.*/, '').replace(/ 0/, '  ');
    var timePart = formatted.match(/\d{2}:\d{2}:\d{2}/)[0];
    formatted = datePart + timePart + ' ' + date.getFullYear() + '\n';
    formatted.split('').forEach(function(chr, index) {
      {{{ makeSetValue('buf', 'index', 'chr.charCodeAt(0)', 'i8') }}}
    });
    {{{ makeSetValue('buf', '25', '0', 'i8') }}}
    return buf;
  },

  ctime__deps: ['localtime', 'asctime'],
  ctime: function(timer) {
    return _asctime(_localtime(timer));
  },

  ctime_r__deps: ['localtime', 'asctime'],
  ctime_r: function(timer, buf) {
    return _asctime_r(_localtime_r(timer, ___tm_current), buf);
  },

  dysize: function(year) {
    var leap = ((year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0)));
    return leap ? 366 : 365;
  },

  // TODO: Initialize these to defaults on startup from system settings.
  // Note: glibc has one fewer underscore for all of these. Also used in other related functions (timegm)
  _tzname: null,
  _daylight: null,
  _timezone: null,
  tzset__deps: ['_tzname', '_daylight', '_timezone'],
  tzset: function() {
    // TODO: Use (malleable) environment variables instead of system settings.
    if (__tzname) return; // glibc does not need the double __

    __timezone = _malloc({{{ Runtime.QUANTUM_SIZE }}});
    {{{ makeSetValue('__timezone', '0', '-(new Date()).getTimezoneOffset() * 60', 'i32') }}}

    __daylight = _malloc({{{ Runtime.QUANTUM_SIZE }}});
    var winter = new Date(2000, 0, 1);
    var summer = new Date(2000, 6, 1);
    {{{ makeSetValue('__daylight', '0', 'Number(winter.getTimezoneOffset() != summer.getTimezoneOffset())', 'i32') }}}

    var winterName = 'GMT'; // XXX do not rely on browser timezone info, it is very unpredictable | winter.toString().match(/\(([A-Z]+)\)/)[1];
    var summerName = 'GMT'; // XXX do not rely on browser timezone info, it is very unpredictable | summer.toString().match(/\(([A-Z]+)\)/)[1];
    var winterNamePtr = allocate(intArrayFromString(winterName), 'i8', ALLOC_NORMAL);
    var summerNamePtr = allocate(intArrayFromString(summerName), 'i8', ALLOC_NORMAL);
    __tzname = _malloc(2 * {{{ Runtime.QUANTUM_SIZE }}}); // glibc does not need the double __
    {{{ makeSetValue('__tzname', '0', 'winterNamePtr', 'i32') }}}
    {{{ makeSetValue('__tzname', Runtime.QUANTUM_SIZE, 'summerNamePtr', 'i32') }}}
  },

  stime__deps: ['$ERRNO_CODES', '__setErrNo'],
  stime: function(when) {
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },

  strftime: function(s, maxsize, format, timeptr) {
    // size_t strftime(char *restrict s, size_t maxsize, const char *restrict format, const struct tm *restrict timeptr);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/strftime.html
    // TODO: Implement.
    return 0;
  },
  strftime_l: 'strftime', // no locale support yet

  strptime: function(buf, format, tm) {
    // char *strptime(const char *restrict buf, const char *restrict format, struct tm *restrict tm);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/strptime.html
    // TODO: Implement.
    return 0;
  },
  strptime_l: 'strptime', // no locale support yet

  getdate: function(string) {
    // struct tm *getdate(const char *string);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/getdate.html
    // TODO: Implement.
    return 0;
  },

  // ==========================================================================
  // sys/time.h
  // ==========================================================================

  __timespec_struct_layout: Runtime.generateStructInfo(['tv_sec', 'tv_nsec'], '%struct.timespec'),
  // TODO: Implement these for real.
  clock_gettime__deps: ['__timespec_struct_layout'],
  clock_gettime: function(clk_id, tp) {
    // int clock_gettime(clockid_t clk_id, struct timespec *tp);
    {{{ makeSetValue('tp', '___timespec_struct_layout.tv_sec', '0', 'i32') }}}
    {{{ makeSetValue('tp', '___timespec_struct_layout.tv_nsec', '0', 'i32') }}}
    return 0;
  },
  clock_settime: function(clk_id, tp) {
    // int clock_settime(clockid_t clk_id, const struct timespec *tp);
    // Nothing.
    return 0;
  },
  clock_getres__deps: ['__timespec_struct_layout'],
  clock_getres: function(clk_id, res) {
    // int clock_getres(clockid_t clk_id, struct timespec *res);
    {{{ makeSetValue('res', '___timespec_struct_layout.tv_sec', '1', 'i32') }}}
    {{{ makeSetValue('res', '___timespec_struct_layout.tv_nsec', '0', 'i32') }}}
    return 0;
  },

  // TODO: Implement remaining functions.
  // http://pubs.opengroup.org/onlinepubs/000095399/basedefs/sys/time.h.html
  gettimeofday: function(ptr) {
    // %struct.timeval = type { i32, i32 }
    var indexes = Runtime.calculateStructAlignment({ fields: ['i32', 'i32'] });
    var now = Date.now();
    {{{ makeSetValue('ptr', 'indexes[0]', 'Math.floor(now/1000)', 'i32') }}} // seconds
    {{{ makeSetValue('ptr', 'indexes[1]', 'Math.floor((now-1000*Math.floor(now/1000))*1000)', 'i32') }}} // microseconds
    return 0;
  },

  // ==========================================================================
  // sys/timeb.h
  // ==========================================================================

  __timeb_struct_layout: Runtime.generateStructInfo([
    ['i32', 'time'],
    ['i16', 'millitm'],
    ['i16', 'timezone'],
    ['i16', 'dstflag']
  ]),
  ftime__deps: ['__timeb_struct_layout'],
  ftime: function(p) {
    var millis = Date.now();
    {{{ makeSetValue('p', '___timeb_struct_layout.time', 'Math.floor(millis/1000)', 'i32') }}};
    {{{ makeSetValue('p', '___timeb_struct_layout.millitm', 'millis % 1000', 'i16') }}};
    {{{ makeSetValue('p', '___timeb_struct_layout.timezone', '0', 'i16') }}}; // TODO
    {{{ makeSetValue('p', '___timeb_struct_layout.dstflag', '0', 'i16') }}}; // TODO
    return 0;
  },

  // ==========================================================================
  // sys/times.h
  // ==========================================================================

  __tms_struct_layout: Runtime.generateStructInfo([
    'tms_utime',
    'tms_stime',
    'tms_cutime',
    'tms_cstime'], '%struct.tms'),
  times__deps: ['__tms_struct_layout', 'memset'],
  times: function(buffer) {
    // clock_t times(struct tms *buffer);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/times.html
    // NOTE: This is fake, since we can't calculate real CPU time usage in JS.
    if (buffer !== 0) {
      _memset(buffer, 0, ___tms_struct_layout.__size__);
    }
    return 0;
  },

  // ==========================================================================
  // sys/types.h
  // ==========================================================================

  // NOTE: These are fake, since we don't support the C device creation API.
  // http://www.kernel.org/doc/man-pages/online/pages/man3/minor.3.html
  makedev: function(maj, min) {
    return 0;
  },
  gnu_dev_makedev: 'makedev',
  major: function(dev) {
    return 0;
  },
  gnu_dev_major: 'major',
  minor: function(dev) {
    return 0;
  },
  gnu_dev_minor: 'minor',

  // ==========================================================================
  // setjmp.h
  //
  // Basic support for setjmp/longjmp: enough to run the wikipedia example and
  // hopefully handle most normal behavior. We do not support cases where
  // longjmp behavior is undefined (for example, if the setjmp function returns
  // before longjmp is called).
  //
  // Note that we need to emulate functions that use setjmp, and also to create
  // a new label we can return to. Emulation make such functions slower, this
  // can be alleviated by making a new function containing just the setjmp
  // related functionality so the slowdown is more limited.
  // ==========================================================================

  setjmp__inline: function(env) {
    // Save the label
    return '(tempInt = setjmpId++, mySetjmpIds[tempInt] = 1, setjmpLabels[tempInt] = label,' + makeSetValue(env, '0', 'tempInt', 'i32') + ', 0)';
  },

  longjmp: function(env, value) {
    throw { longjmp: true, id: {{{ makeGetValue('env', '0', 'i32') }}}, value: value || 1 };
  },

  // ==========================================================================
  // signal.h
  // ==========================================================================

  signal: function(sig, func) {
    // TODO
    return 0;
  },
  sigemptyset: function(set) {
    // int sigemptyset(sigset_t *set);
    // TODO: Implement for real; don't hardcode offsets.
    {{{ makeSetValue('set', '0', '0', 'i32') }}}
    {{{ makeSetValue('set', '4', '0', 'i32') }}}
    {{{ makeSetValue('set', '8', '0', 'i32') }}}
    {{{ makeSetValue('set', '12', '0', 'i32') }}}
    return 0;
  },
  sigfillset: 'sigemptyset',
  sigdelset: 'sigemptyset',
  sigaction: function(set) {
    // int sigemptyset(sigset_t *set);
    // TODO: Implement for real.
    return 0;
  },
  sigprocmask: 'sigaction',
  __libc_current_sigrtmin: function() {
    return 0;
  },
  __libc_current_sigrtmax: function() {
    return 0;
  },
  kill__deps: ['$ERRNO_CODES', '__setErrNo'],
  kill: function(pid, sig) {
    // int kill(pid_t pid, int sig);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/kill.html
    // Makes no sense in a single-process environment.
    ___setErrNo(ERRNO_CODES.EPERM);
    return -1;
  },
  killpg: 'kill',

  // ==========================================================================
  // sys/wait.h
  // ==========================================================================

  wait__deps: ['$ERRNO_CODES', '__setErrNo'],
  wait: function(stat_loc) {
    // pid_t wait(int *stat_loc);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/wait.html
    // Makes no sense in a single-process environment.
    ___setErrNo(ERRNO_CODES.ECHILD);
    return -1;
  },
  // NOTE: These aren't really the same, but we use the same stub for them all.
  waitid: 'wait',
  waitpid: 'wait',
  wait3: 'wait',
  wait4: 'wait',

  // ==========================================================================
  // locale.h
  // ==========================================================================

  newlocale: function(mask, locale, base) {
    return 0;
  },

  uselocale: function(locale) {
    return 0;
  },

  setlocale: function(category, locale) {
    if (!_setlocale.ret) _setlocale.ret = allocate([0], 'i8', ALLOC_NORMAL);
    return _setlocale.ret;
  },

  localeconv: function() {
    // %struct.timeval = type { char* decimal point, other stuff... }
    // var indexes = Runtime.calculateStructAlignment({ fields: ['i32', 'i32'] });
    var me = _localeconv;
    if (!me.ret) {
      me.ret = allocate([allocate(intArrayFromString('.'), 'i8', ALLOC_NORMAL)], 'i8*', ALLOC_NORMAL); // just decimal point, for now
    }
    return me.ret;
  },

  // ==========================================================================
  // langinfo.h
  // ==========================================================================

  nl_langinfo: function(item) {
    // char *nl_langinfo(nl_item item);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/nl_langinfo.html
    var result;
    switch (item) {
      case {{{ cDefine('CODESET') }}}:
        result = 'ANSI_X3.4-1968';
        break;
      case {{{ cDefine('D_T_FMT') }}}:
        result = '%a %b %e %H:%M:%S %Y';
        break;
      case {{{ cDefine('D_FMT') }}}:
        result = '%m/%d/%y';
        break;
      case {{{ cDefine('T_FMT') }}}:
        result = '%H:%M:%S';
        break;
      case {{{ cDefine('T_FMT_AMPM') }}}:
        result = '%I:%M:%S %p';
        break;
      case {{{ cDefine('AM_STR') }}}:
        result = 'AM';
        break;
      case {{{ cDefine('PM_STR') }}}:
        result = 'PM';
        break;
      case {{{ cDefine('DAY_1') }}}:
        result = 'Sunday';
        break;
      case {{{ cDefine('DAY_2') }}}:
        result = 'Monday';
        break;
      case {{{ cDefine('DAY_3') }}}:
        result = 'Tuesday';
        break;
      case {{{ cDefine('DAY_4') }}}:
        result = 'Wednesday';
        break;
      case {{{ cDefine('DAY_5') }}}:
        result = 'Thursday';
        break;
      case {{{ cDefine('DAY_6') }}}:
        result = 'Friday';
        break;
      case {{{ cDefine('DAY_7') }}}:
        result = 'Saturday';
        break;
      case {{{ cDefine('ABDAY_1') }}}:
        result = 'Sun';
        break;
      case {{{ cDefine('ABDAY_2') }}}:
        result = 'Mon';
        break;
      case {{{ cDefine('ABDAY_3') }}}:
        result = 'Tue';
        break;
      case {{{ cDefine('ABDAY_4') }}}:
        result = 'Wed';
        break;
      case {{{ cDefine('ABDAY_5') }}}:
        result = 'Thu';
        break;
      case {{{ cDefine('ABDAY_6') }}}:
        result = 'Fri';
        break;
      case {{{ cDefine('ABDAY_7') }}}:
        result = 'Sat';
        break;
      case {{{ cDefine('MON_1') }}}:
        result = 'January';
        break;
      case {{{ cDefine('MON_2') }}}:
        result = 'February';
        break;
      case {{{ cDefine('MON_3') }}}:
        result = 'March';
        break;
      case {{{ cDefine('MON_4') }}}:
        result = 'April';
        break;
      case {{{ cDefine('MON_5') }}}:
        result = 'May';
        break;
      case {{{ cDefine('MON_6') }}}:
        result = 'June';
        break;
      case {{{ cDefine('MON_7') }}}:
        result = 'July';
        break;
      case {{{ cDefine('MON_8') }}}:
        result = 'August';
        break;
      case {{{ cDefine('MON_9') }}}:
        result = 'September';
        break;
      case {{{ cDefine('MON_10') }}}:
        result = 'October';
        break;
      case {{{ cDefine('MON_11') }}}:
        result = 'November';
        break;
      case {{{ cDefine('MON_12') }}}:
        result = 'December';
        break;
      case {{{ cDefine('ABMON_1') }}}:
        result = 'Jan';
        break;
      case {{{ cDefine('ABMON_2') }}}:
        result = 'Feb';
        break;
      case {{{ cDefine('ABMON_3') }}}:
        result = 'Mar';
        break;
      case {{{ cDefine('ABMON_4') }}}:
        result = 'Apr';
        break;
      case {{{ cDefine('ABMON_5') }}}:
        result = 'May';
        break;
      case {{{ cDefine('ABMON_6') }}}:
        result = 'Jun';
        break;
      case {{{ cDefine('ABMON_7') }}}:
        result = 'Jul';
        break;
      case {{{ cDefine('ABMON_8') }}}:
        result = 'Aug';
        break;
      case {{{ cDefine('ABMON_9') }}}:
        result = 'Sep';
        break;
      case {{{ cDefine('ABMON_10') }}}:
        result = 'Oct';
        break;
      case {{{ cDefine('ABMON_11') }}}:
        result = 'Nov';
        break;
      case {{{ cDefine('ABMON_12') }}}:
        result = 'Dec';
        break;
      case {{{ cDefine('ALT_DIGITS') }}}:
        result = '';
        break;
      case {{{ cDefine('RADIXCHAR') }}}:
        result = '.';
        break;
      case {{{ cDefine('THOUSEP') }}}:
        result = '';
        break;
      case {{{ cDefine('YESEXPR') }}}:
        result = '^[yY]';
        break;
      case {{{ cDefine('NOEXPR') }}}:
        result = '^[nN]';
        break;
      case {{{ cDefine('CRNCYSTR') }}}:
        result = '-';
        break;
      case {{{ cDefine('ERA') }}}:
      case {{{ cDefine('ERA_D_FMT') }}}:
      case {{{ cDefine('ERA_D_T_FMT') }}}:
      case {{{ cDefine('ERA_T_FMT') }}}:
      default:
        result = '';
        break;
    }

    var me = _nl_langinfo;
    if (!me.ret) me.ret = _malloc(32);
    for (var i = 0; i < result.length; i++) {
      {{{ makeSetValue('me.ret', 'i', 'result.charCodeAt(i)', 'i8') }}}
    }
    {{{ makeSetValue('me.ret', 'i', '0', 'i8') }}}
    return me.ret;
  },

  // ==========================================================================
  // errno.h
  // ==========================================================================

  $ERRNO_CODES: {
    E2BIG: 7,
    EACCES: 13,
    EADDRINUSE: 98,
    EADDRNOTAVAIL: 99,
    EAFNOSUPPORT: 97,
    EAGAIN: 11,
    EALREADY: 114,
    EBADF: 9,
    EBADMSG: 74,
    EBUSY: 16,
    ECANCELED: 125,
    ECHILD: 10,
    ECONNABORTED: 103,
    ECONNREFUSED: 111,
    ECONNRESET: 104,
    EDEADLK: 35,
    EDESTADDRREQ: 89,
    EDOM: 33,
    EDQUOT: 122,
    EEXIST: 17,
    EFAULT: 14,
    EFBIG: 27,
    EHOSTUNREACH: 113,
    EIDRM: 43,
    EILSEQ: 84,
    EINPROGRESS: 115,
    EINTR: 4,
    EINVAL: 22,
    EIO: 5,
    EISCONN: 106,
    EISDIR: 21,
    ELOOP: 40,
    EMFILE: 24,
    EMLINK: 31,
    EMSGSIZE: 90,
    EMULTIHOP: 72,
    ENAMETOOLONG: 36,
    ENETDOWN: 100,
    ENETRESET: 102,
    ENETUNREACH: 101,
    ENFILE: 23,
    ENOBUFS: 105,
    ENODATA: 61,
    ENODEV: 19,
    ENOENT: 2,
    ENOEXEC: 8,
    ENOLCK: 37,
    ENOLINK: 67,
    ENOMEM: 12,
    ENOMSG: 42,
    ENOPROTOOPT: 92,
    ENOSPC: 28,
    ENOSR: 63,
    ENOSTR: 60,
    ENOSYS: 38,
    ENOTCONN: 107,
    ENOTDIR: 20,
    ENOTEMPTY: 39,
    ENOTRECOVERABLE: 131,
    ENOTSOCK: 88,
    ENOTSUP: 95,
    ENOTTY: 25,
    ENXIO: 6,
    EOVERFLOW: 75,
    EOWNERDEAD: 130,
    EPERM: 1,
    EPIPE: 32,
    EPROTO: 71,
    EPROTONOSUPPORT: 93,
    EPROTOTYPE: 91,
    ERANGE: 34,
    EROFS: 30,
    ESPIPE: 29,
    ESRCH: 3,
    ESTALE: 116,
    ETIME: 62,
    ETIMEDOUT: 110,
    ETXTBSY: 26,
    EWOULDBLOCK: 11,
    EXDEV: 18,
  },
  $ERRNO_MESSAGES: {
    2: 'No such file or directory',
    13: 'Permission denied',
    98: 'Address already in use',
    99: 'Cannot assign requested address',
    97: 'Address family not supported by protocol',
    11: 'Resource temporarily unavailable',
    114: 'Operation already in progress',
    9: 'Bad file descriptor',
    74: 'Bad message',
    16: 'Device or resource busy',
    125: 'Operation canceled',
    10: 'No child processes',
    103: 'Software caused connection abort',
    111: 'Connection refused',
    104: 'Connection reset by peer',
    35: 'Resource deadlock avoided',
    89: 'Destination address required',
    33: 'Numerical argument out of domain',
    122: 'Disk quota exceeded',
    17: 'File exists',
    14: 'Bad address',
    27: 'File too large',
    113: 'No route to host',
    43: 'Identifier removed',
    84: 'Invalid or incomplete multibyte or wide character',
    115: 'Operation now in progress',
    4: 'Interrupted system call',
    22: 'Invalid argument',
    5: 'Input/output error',
    106: 'Transport endpoint is already connected',
    21: 'Is a directory',
    40: 'Too many levels of symbolic links',
    24: 'Too many open files',
    31: 'Too many links',
    90: 'Message too long',
    72: 'Multihop attempted',
    36: 'File name too long',
    100: 'Network is down',
    102: 'Network dropped connection on reset',
    101: 'Network is unreachable',
    23: 'Too many open files in system',
    105: 'No buffer space available',
    61: 'No data available',
    19: 'No such device',
    8: 'Exec format error',
    37: 'No locks available',
    67: 'Link has been severed',
    12: 'Cannot allocate memory',
    42: 'No message of desired type',
    92: 'Protocol not available',
    28: 'No space left on device',
    63: 'Out of streams resources',
    60: 'Device not a stream',
    38: 'Function not implemented',
    107: 'Transport endpoint is not connected',
    20: 'Not a directory',
    39: 'Directory not empty',
    131: 'State not recoverable',
    88: 'Socket operation on non-socket',
    95: 'Operation not supported',
    25: 'Inappropriate ioctl for device',
    6: 'No such device or address',
    75: 'Value too large for defined data type',
    130: 'Owner died',
    1: 'Operation not permitted',
    32: 'Broken pipe',
    71: 'Protocol error',
    93: 'Protocol not supported',
    91: 'Protocol wrong type for socket',
    34: 'Numerical result out of range',
    30: 'Read-only file system',
    29: 'Illegal seek',
    3: 'No such process',
    116: 'Stale NFS file handle',
    62: 'Timer expired',
    110: 'Connection timed out',
    26: 'Text file busy',
    18: 'Invalid cross-device link'
  },
  __setErrNo__postset: '___setErrNo(0);',
  __setErrNo: function(value) {
    // For convenient setting and returning of errno.
    if (!___setErrNo.ret) ___setErrNo.ret = allocate([0], 'i32', ALLOC_STATIC);
    {{{ makeSetValue('___setErrNo.ret', '0', 'value', 'i32') }}}
    return value;
  },
  __errno_location__deps: ['__setErrNo'],
  __errno_location: function() {
    return ___setErrNo.ret;
  },
  __errno: '__errno_location',

  // ==========================================================================
  // sys/resource.h
  // ==========================================================================

  // TODO: Implement for real.
  __rlimit_struct_layout: Runtime.generateStructInfo(['rlim_cur', 'rlim_max'], '%struct.rlimit'),
  getrlimit__deps: ['__rlimit_struct_layout'],
  getrlimit: function(resource, rlp) {
    // int getrlimit(int resource, struct rlimit *rlp);
    {{{ makeSetValue('rlp', '___rlimit_struct_layout.rlim_cur', '-1', 'i32') }}}  // RLIM_INFINITY
    {{{ makeSetValue('rlp', '___rlimit_struct_layout.rlim_max', '-1', 'i32') }}}  // RLIM_INFINITY
    return 0;
  },
  setrlimit: function(resource, rlp) {
    // int setrlimit(int resource, const struct rlimit *rlp)
    return 0;
  },
  __01getrlimit64_: 'getrlimit',

  // TODO: Implement for real. We just do time used, and no useful data
  __rusage_struct_layout: Runtime.generateStructInfo([
    'ru_utime',
    'ru_stime',
    'ru_maxrss',
    'ru_ixrss',
    'ru_idrss',
    'ru_isrss',
    'ru_minflt',
    'ru_majflt',
    'ru_nswap',
    'ru_inblock',
    'ru_oublock',
    'ru_msgsnd',
    'ru_msgrcv',
    'ru_nsignals',
    'ru_nvcsw',
    'ru_nivcsw'], '%struct.rusage'),
  getrusage__deps: ['__rusage_struct_layout'],
  getrusage: function(resource, rlp) {
    // %struct.timeval = type { i32, i32 }
    var timeval = Runtime.calculateStructAlignment({ fields: ['i32', 'i32'] });

    // int getrusage(int resource, struct rusage *rlp);
    {{{ makeSetValue('rlp', '___rusage_struct_layout.ru_utime+timeval[0]', '1', 'i32') }}}
    {{{ makeSetValue('rlp', '___rusage_struct_layout.ru_utime+timeval[1]', '2', 'i32') }}}
    {{{ makeSetValue('rlp', '___rusage_struct_layout.ru_stime+timeval[0]', '3', 'i32') }}}
    {{{ makeSetValue('rlp', '___rusage_struct_layout.ru_stime+timeval[1]', '4', 'i32') }}}
    return 0;
  },

  // ==========================================================================
  // pthread.h (stubs for mutexes only - no thread support yet!)
  // ==========================================================================

  pthread_mutex_init: function() {},
  pthread_mutex_destroy: function() {},
  pthread_mutexattr_init: function() {},
  pthread_mutexattr_settype: function() {},
  pthread_mutexattr_destroy: function() {},
  pthread_mutex_lock: function() {},
  pthread_mutex_unlock: function() {},
  pthread_cond_broadcast: function() {},
  pthread_self: function() {
    //FIXME: assumes only a single thread
    return 0;
  },
  pthread_attr_init: function(attr) {
    /* int pthread_attr_init(pthread_attr_t *attr); */
    //FIXME: should allocate a pthread_attr_t
    return 0;
  },
  pthread_getattr_np: function(thread, attr) {
    /* int pthread_getattr_np(pthread_t thread, pthread_attr_t *attr); */
    //FIXME: should fill in attributes of the given thread in pthread_attr_t
    return 0;
  },
  pthread_attr_destroy: function(attr) {
    /* int pthread_attr_destroy(pthread_attr_t *attr); */
    //FIXME: should destroy the pthread_attr_t struct
    return 0;
  },
  pthread_attr_getstack: function(attr, stackaddr, stacksize) {
    /* int pthread_attr_getstack(const pthread_attr_t *restrict attr,
       void **restrict stackaddr, size_t *restrict stacksize); */
    /*FIXME: assumes that there is only one thread, and that attr is the
      current thread*/
    {{{ makeSetValue('stackaddr', '0', 'STACK_ROOT', 'i8*') }}}
    {{{ makeSetValue('stacksize', '0', 'TOTAL_STACK', 'i32') }}}
    return 0;
  },

  pthread_once: function(ptr, func) {
    if (!_pthread_once.seen) _pthread_once.seen = {};
    if (ptr in _pthread_once.seen) return;
    FUNCTION_TABLE[func]();
    _pthread_once.seen[ptr] = 1;
  },

  pthread_key_create: function(key, destructor) {
    if (!_pthread_key_create.keys) _pthread_key_create.keys = {};
    _pthread_key_create.keys[key] = null;
  },

  pthread_getspecific: function(key) {
    return _pthread_key_create.keys[key];
  },

  pthread_setspecific: function(key, value) {
    _pthread_key_create.keys[key] = value;
  },

  pthread_cleanup_push: function(routine, arg) {
    __ATEXIT__.push({ func: function() { FUNCTION_TABLE[routine](arg) } })
    _pthread_cleanup_push.level = __ATEXIT__.length;
  },

  pthread_cleanup_pop: function() {
    assert(_pthread_cleanup_push.level == __ATEXIT__.length, 'cannot pop if something else added meanwhile!');
    __ATEXIT__.pop();
    _pthread_cleanup_push.level = __ATEXIT__.length;
  },

  // ==========================================================================
  // malloc.h
  // ==========================================================================

  memalign: function(boundary, size) {
    // leaks, and even returns an invalid pointer. Horrible hack... but then, this is a deprecated function...
    var ret = Runtime.staticAlloc(size + boundary);
    return ret + boundary - (ret % boundary);
  },

  posix_memalign__deps: ['memalign'],
  posix_memalign: function(memptr, alignment, size) {
    var ptr = _memalign(alignment, size);
    {{{ makeSetValue('memptr', '0', 'ptr', 'i8*') }}}
    return 0;
  },

  // ==========================================================================
  // arpa/inet.h
  // ==========================================================================

  htonl: function(value) {
    return ((value & 0xff) << 24) + ((value & 0xff00) << 8) +
           ((value & 0xff0000) >>> 8) + ((value & 0xff000000) >>> 24);
  },
  htons: function(value) {
    return ((value & 0xff) << 8) + ((value & 0xff00) >> 8);
  },
  ntohl: 'htonl',
  ntohs: 'htons',

  inet_addr: function(ptr) {
    var b = Pointer_stringify(ptr).split(".");
    if (b.length !== 4) return -1; // we return -1 for error, and otherwise a uint32. this helps inet_pton differentiate
    return (Number(b[0]) | (Number(b[1]) << 8) | (Number(b[2]) << 16) | (Number(b[3]) << 24)) >>> 0;
  },

  inet_pton__deps: ['__setErrNo', '$ERRNO_CODES', 'inet_addr'],
  inet_pton: function(af, src, dst) {
    // int af, const char *src, void *dst
    if ((af ^ {{{ cDefine("AF_INET") }}}) !==  0) { ___setErrNo(ERRNO_CODES.EAFNOSUPPORT); return -1; }
    var ret = _inet_addr(src);
    if (ret == -1 || isNaN(ret)) return 0;
    setValue(dst, ret, 'i32');
    return 1;
  },

  _inet_ntop_raw: function(addr) {
    return (addr & 0xff) + '.' + ((addr >> 8) & 0xff) + '.' + ((addr >> 16) & 0xff) + '.' + ((addr >> 24) & 0xff)
  },

  inet_ntop__deps: ['_inet_ntop_raw'],
  inet_ntop: function(af, src, dst, size) {
    var addr = getValue(src, 'i32');
    var str = __inet_ntop_raw(addr);
    writeStringToMemory(str.substr(0, size), dst);
    return dst;
  },

  inet_ntoa__deps: ['inet_ntop'],
  inet_ntoa: function(in_addr) {
    if (!_inet_ntoa.buffer) {
      _inet_ntoa.buffer = _malloc(1024);
    }
    return _inet_ntop(0, in_addr, _inet_ntoa.buffer, 1024);
  },

  inet_aton__deps: ['inet_addr'],
  inet_aton: function(cp, inp) {
    var addr = _inet_addr(cp);
    setValue(inp, addr, 'i32');
    if (addr < 0) return 0;
    return 1;
  },

  // ==========================================================================
  // netdb.h
  // ==========================================================================

  // All we can do is alias names to ips. you give this a name, it returns an
  // "ip" that we later know to use as a name. There is no way to do actual
  // name resolving clientside in a browser.
  // we do the aliasing in 172.29.*.*, giving us 65536 possibilities
  // note: lots of leaking here!
  __hostent_struct_layout: Runtime.generateStructInfo([
    ['i8*', 'h_name'],
    ['i8**', 'h_aliases'],
    ['i32', 'h_addrtype'],
    ['i32', 'h_length'],
    ['i8**', 'h_addr_list'],
  ]),
  gethostbyname__deps: ['__hostent_struct_layout'],
  gethostbyname: function(name) {
    name = Pointer_stringify(name);
      if (!_gethostbyname.id) {
        _gethostbyname.id = 1;
        _gethostbyname.table = {};
      }
    var id = _gethostbyname.id++;
    assert(id < 65535);
    var fakeAddr = 172 | (29 << 8) | ((id & 0xff) << 16) | ((id & 0xff00) << 24);
    _gethostbyname.table[id] = name;
    // generate hostent
    var ret = _malloc(___hostent_struct_layout.__size__);
    var nameBuf = _malloc(name.length+1);
    writeStringToMemory(name, nameBuf);
    setValue(ret+___hostent_struct_layout.h_name, nameBuf, 'i8*');
    var aliasesBuf = _malloc(4);
    setValue(aliasesBuf, 0, 'i8*');
    setValue(ret+___hostent_struct_layout.h_aliases, aliasesBuf, 'i8**');
    setValue(ret+___hostent_struct_layout.h_addrtype, {{{ cDefine("AF_INET") }}}, 'i32');
    setValue(ret+___hostent_struct_layout.h_length, 4, 'i32');
    var addrListBuf = _malloc(12);
    setValue(addrListBuf, addrListBuf+8, 'i32*');
    setValue(addrListBuf+4, 0, 'i32*');
    setValue(addrListBuf+8, fakeAddr, 'i32');
    setValue(ret+___hostent_struct_layout.h_addr_list, addrListBuf, 'i8**');
    return ret;
  },

  gethostbyname_r__deps: ['gethostbyname'],
  gethostbyname_r: function(name, hostData, buffer, bufferSize, hostEntry, errnum) {
    var data = _gethostbyname(name);
    _memcpy(hostData, data, ___hostent_struct_layout.__size__);
    _free(data);
    setValue(errnum, 0, 'i32');
    return 0;
  },

  // ==========================================================================
  // sockets. Note that the implementation assumes all sockets are always
  // nonblocking
  // ==========================================================================

  $Sockets__deps: ['__setErrNo', '$ERRNO_CODES'],
  $Sockets: {
    BUFFER_SIZE: 10*1024, // initial size
    MAX_BUFFER_SIZE: 10*1024*1024, // maximum size we will grow the buffer
    nextFd: 1,
    fds: {},
    sockaddr_in_layout: Runtime.generateStructInfo([
      ['i16', 'sin_family'],
      ['i16', 'sin_port'],
      ['i32', 'sin_addr'],
      ['i64', 'sin_zero'],
    ]),
    msghdr_layout: Runtime.generateStructInfo([
      ['*', 'msg_name'],
      ['i32', 'msg_namelen'],
      ['*', 'msg_iov'],
      ['i32', 'msg_iovlen'],
      ['*', 'msg_control'],
      ['i32', 'msg_controllen'],
      ['i32', 'msg_flags'],
    ]),
  },

  socket__deps: ['$Sockets'],
  socket: function(family, type, protocol) {
    var fd = Sockets.nextFd++;
    Sockets.fds[fd] = {
      connected: false
    };
    return fd;
  },

  connect__deps: ['$Sockets', '_inet_ntop_raw', 'ntohs', 'gethostbyname'],
  connect: function(fd, addr, addrlen) {
    var info = Sockets.fds[fd];
    if (!info) return -1;
    info.connected = true;
    info.addr = getValue(addr + Sockets.sockaddr_in_layout.sin_addr, 'i32');
    info.port = _ntohs(getValue(addr + Sockets.sockaddr_in_layout.sin_port, 'i16'));
    info.host = __inet_ntop_raw(info.addr);
    // Support 'fake' ips from gethostbyname
    var parts = info.host.split('.');
    if (parts[0] == '172' && parts[1] == '29') {
      var low = Number(parts[2]);
      var high = Number(parts[3]);
      info.host = _gethostbyname.table[low + 0xff*high];
      assert(info.host, 'problem translating fake ip ' + parts);
    }
    console.log('opening ws://' + info.host + ':' + info.port);
    info.socket = new WebSocket('ws://' + info.host + ':' + info.port, ['binary']);
    info.socket.binaryType = 'arraybuffer';
    info.buffer = new Uint8Array(Sockets.BUFFER_SIZE);
    info.bufferWrite = info.bufferRead = 0;
    info.socket.onmessage = function (event) {
      assert(typeof event.data !== 'string' && event.data.byteLength); // must get binary data!
      var data = new Uint8Array(event.data); // make a typed array view on the array buffer
      var len = data.length;
#if SOCKET_DEBUG
      Module.print(['onmessage', window.location, data, len, '|', Array.prototype.slice.call(data)]);
#endif
      for (var i = 0; i < len; i++) { // TODO: typed array set, carefully with ranges, or other trick
        info.buffer[info.bufferWrite++] = data[i];
        if (info.bufferWrite == info.buffer.length) info.bufferWrite = 0;
        if (info.bufferWrite == info.bufferRead) {
          // grow the buffer
          var currLen = info.buffer.length;
          if (currLen > Sockets.MAX_BUFFER_SIZE) throw 'socket buffer overflow';
          var newBuffer = new Uint8Array(currLen*2);
          for (var j = 0; j < currLen; j++) {
            newBuffer[j] = info.buffer[(info.bufferRead + j)%currLen];
          }
          info.bufferRead = 0;
          info.bufferWrite = currLen;
          info.buffer = newBuffer;
        }
      }
    }
    info.sendQueue = new Uint8Array(1024);
    info.sendQueueUsed = 0;
    info.senderWaiting = false;
    info.sender = function(data, justQueue) {
      if (data) {
#if SOCKET_DEBUG
        Module.print(['sender', data, data.length, '|', Array.prototype.slice.call(data)]);
#endif
        if (info.sendQueueUsed + data.length >= info.sendQueue.length) {
          var newQueue = new Uint8Array(2*Math.max(info.sendQueue.length, data.length));
          newQueue.set(info.sendQueue);
          info.sendQueue = newQueue;
        }
        info.sendQueue.set(data, info.sendQueueUsed); // must copy, because while this waits memory can change!
        info.sendQueueUsed += data.length;
      } else {
        info.senderWaiting = false; // we are a setTimeout callback
        if (info.sendQueueUsed == 0) return;
      }
      if (info.socket.readyState != info.socket.OPEN) {
        if (!info.senderWaiting) {
          console.log('waiting for socket in order to send');
          setTimeout(info.sender, 100);
          info.senderWaiting = true;
        }
        return;
      }
      if (justQueue) return;
#if SOCKET_DEBUG
      Module.print('sender actually sending ' + info.sendQueueUsed);
#endif
      info.socket.send(new Uint8Array(info.sendQueue.subarray(0, info.sendQueueUsed)).buffer); // TODO: if browser accepts views, can optimize this
      info.sendQueueUsed = 0;
    };
    return 0;
  },

  recv__deps: ['$Sockets'],
  recv: function(fd, buf, len, flags) {
    var info = Sockets.fds[fd];
    if (!info) return -1;
    if (info.bufferWrite == info.bufferRead) {
      ___setErrNo(ERRNO_CODES.EAGAIN); // no data, and all sockets are nonblocking, so this is the right behavior
      return 0; // should this be -1 like the spec says?
    }
    var ret = 0;
#if SOCKET_DEBUG
    Module.print('pre-recv: ' + [len, info.bufferWrite, info.bufferRead]);
#endif
    while (info.bufferWrite != info.bufferRead && len > 0) {
      // write out a byte
      {{{ makeSetValue('buf++', '0', 'info.buffer[info.bufferRead++]', 'i8') }}};
      if (info.bufferRead == info.buffer.length) info.bufferRead = 0;
      len--;
      ret++;
    }
#if SOCKET_DEBUG
    Module.print('recv: ' + [ret, len, buf] + ' : ' + Array.prototype.slice.call(HEAPU8.subarray(buf-ret, buf)));
#endif
    return ret;
  },

  send__deps: ['$Sockets'],
  send: function(fd, buf, len, flags) {
    var info = Sockets.fds[fd];
    if (!info) return -1;
    info.sender(HEAPU8.subarray(buf, buf+len));
    return len;
  },

  sendmsg__deps: ['$Sockets', 'connect'],
  sendmsg: function(fd, msg, flags) {
    var info = Sockets.fds[fd];
    if (!info) return -1;
    // if we are not connected, use the address info in the message
    if (!info.connected) {
      var name = {{{ makeGetValue('msg', 'Sockets.msghdr_layout.msg_name', '*') }}};
      assert(name, 'sendmsg on non-connected socket, and no name/address in the message');
      _connect(fd, name, {{{ makeGetValue('msg', 'Sockets.msghdr_layout.msg_namelen', 'i32') }}});
    }
    var iov = {{{ makeGetValue('msg', 'Sockets.msghdr_layout.msg_iov', 'i8*') }}};
    var num = {{{ makeGetValue('msg', 'Sockets.msghdr_layout.msg_iovlen', 'i32') }}};
    var ret = 0;
#if SOCKET_DEBUG
      Module.print('sendmsg vecs: ' + num);
#endif
    for (var i = 0; i < num; i++) {
      var currNum = {{{ makeGetValue('iov', '8*i + 4', 'i32') }}};
#if SOCKET_DEBUG
      Module.print('sendmsg curr size: ' + currNum);
#endif
      if (!currNum) continue;
      var currBuf = {{{ makeGetValue('iov', '8*i', 'i8*') }}};
      info.sender(HEAPU8.subarray(currBuf, currBuf+currNum), true);
      ret += currNum;
    }
    info.sender(null); // flush all of these together. Important they get sent as a single socket message
    return ret;
  },

  recvmsg__deps: ['$Sockets', 'connect', 'recv', '__setErrNo', '$ERRNO_CODES', 'htons'],
  recvmsg: function(fd, msg, flags) {
    var info = Sockets.fds[fd];
    if (!info) return -1;
    // if we are not connected, use the address info in the message
    if (!info.connected) {
#if SOCKET_DEBUG
      Module.print('recvmsg connecting');
#endif
      var name = {{{ makeGetValue('msg', 'Sockets.msghdr_layout.msg_name', '*') }}};
      assert(name, 'sendmsg on non-connected socket, and no name/address in the message');
      _connect(fd, name, {{{ makeGetValue('msg', 'Sockets.msghdr_layout.msg_namelen', 'i32') }}});
    }
    var bytes = info.bufferWrite - info.bufferRead;
    if (bytes < 0) bytes += info.buffer.length;
    if (bytes == 0) {
      ___setErrNo(ERRNO_CODES.EWOULDBLOCK);
      return -1;
    }
#if SOCKET_DEBUG
    Module.print('recvmsg bytes: ' + bytes);
#endif
    // write source
    var name = {{{ makeGetValue('msg', 'Sockets.msghdr_layout.msg_name', '*') }}};
    {{{ makeSetValue('name', 'Sockets.sockaddr_in_layout.sin_addr', 'info.addr', 'i32') }}};
    {{{ makeSetValue('name', 'Sockets.sockaddr_in_layout.sin_port', '_htons(info.port)', 'i16') }}};
    // write data
    var ret = bytes;
    var iov = {{{ makeGetValue('msg', 'Sockets.msghdr_layout.msg_iov', 'i8*') }}};
    var num = {{{ makeGetValue('msg', 'Sockets.msghdr_layout.msg_iovlen', 'i32') }}};
    var data = '';
    for (var i = 0; i < num && bytes > 0; i++) {
      var currNum = {{{ makeGetValue('iov', '8*i + 4', 'i32') }}};
#if SOCKET_DEBUG
      Module.print('recvmsg loop ' + [i, num, bytes, currNum]);
#endif
      if (!currNum) continue;
      currNum = Math.min(currNum, bytes); // XXX what should happen when we partially fill a buffer..?
      bytes -= currNum;
      var currBuf = {{{ makeGetValue('iov', '8*i', 'i8*') }}};
#if SOCKET_DEBUG
      Module.print('recvmsg call recv ' + currNum);
#endif
      assert(_recv(fd, currBuf, currNum, 0) == currNum);
    }
    return ret;
  },

  recvfrom__deps: ['$Sockets', 'connect', 'recv'],
  recvfrom: function(fd, buf, len, flags, addr, addrlen) {
    var info = Sockets.fds[fd];
    if (!info) return -1;
    // if we are not connected, use the address info in the message
    if (!info.connected) {
      //var name = {{{ makeGetValue('addr', '0', '*') }}};
      _connect(fd, addr, addrlen);
    }
    return _recv(fd, buf, len, flags);
  },

  shutdown: function(fd, how) {
    var info = Sockets.fds[fd];
    if (!info) return -1;
    info.socket.close();
    Sockets.fds[fd] = null;
  },

  ioctl: function(fd, request, varargs) {
    var info = Sockets.fds[fd];
    if (!info) return -1;
    var start = info.bufferRead;
    var end = info.bufferWrite;
    if (end < start) end += info.buffer.length;
    var dest = {{{ makeGetValue('varargs', '0', 'i32') }}};
    {{{ makeSetValue('dest', '0', 'end - start', 'i32') }}};
    return 0;
  },

  setsockopt: function(d, level, optname, optval, optlen) {
    console.log('ignoring setsockopt command');
    return 0;
  },

  bind__deps: ['connect'],
  bind: function(fd, addr, addrlen) {
    return _connect(fd, addr, addrlen);
  },

  listen: function(fd, backlog) {
    return 0;
  },

  accept: function(fd, addr, addrlen) {
    // TODO: webrtc queued incoming connections, etc.
    // For now, the model is that bind does a connect, and we "accept" that one connection,
    // which has host:port the same as ours. We also return the same socket fd.
    var info = Sockets.fds[fd];
    if (!info) return -1;
    if (addr) {
      setValue(addr + Sockets.sockaddr_in_layout.sin_addr, info.addr, 'i32');
      setValue(addr + Sockets.sockaddr_in_layout.sin_port, info.port, 'i32');
      setValue(addrlen, Sockets.sockaddr_in_layout.__size__, 'i32');
    }
    return fd;
  },

  // ==========================================================================
  // emscripten.h
  // ==========================================================================

  emscripten_run_script: function(ptr) {
    eval(Pointer_stringify(ptr));
  },

  emscripten_run_script_int: function(ptr) {
    return eval(Pointer_stringify(ptr));
  },

  emscripten_run_script_string: function(ptr) {
    var s = eval(Pointer_stringify(ptr));
    var me = _emscripten_run_script_string;
    if (!me.bufferSize || me.bufferSize < s.length+1) {
      if (me.bufferSize) _free(me.buffer);
      me.bufferSize = s.length+1;
      me.buffer = _malloc(me.bufferSize);
    }
    writeStringToMemory(s, me.buffer);
    return me.buffer;
  },

  emscripten_random: function() {
    return Math.random();
  },

  $Profiling: {
    max_: 0,
    times: null,
    invalid: 0,
    dump: function() {
      if (Profiling.invalid) {
        Module.printErr('Invalid # of calls to Profiling begin and end!');
        return;
      }
      Module.printErr('Profiling data:')
      for (var i = 0; i < Profiling.max_; i++) {
        Module.printErr('Block ' + i + ': ' + Profiling.times[i]);
      }
    }
  },
  EMSCRIPTEN_PROFILE_INIT__deps: ['$Profiling'],
  EMSCRIPTEN_PROFILE_INIT: function(max_) {
    Profiling.max_ = max_;
    Profiling.times = new Array(max_);
    for (var i = 0; i < max_; i++) Profiling.times[i] = 0;
  },
  EMSCRIPTEN_PROFILE_BEGIN__inline: function(id) {
    return 'Profiling.times[' + id + '] -= Date.now();'
         + 'Profiling.invalid++;'
  },
  EMSCRIPTEN_PROFILE_END__inline: function(id) {
    return 'Profiling.times[' + id + '] += Date.now();'
         + 'Profiling.invalid--;'
  }
};

function autoAddDeps(object, name) {
  name = [name];
  for (var item in object) {
    if (item.substr(-6) != '__deps' && !object[item + '__deps']) {
      object[item + '__deps'] = name;
    }
  }
}

