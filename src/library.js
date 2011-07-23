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

  $FS__deps: ['$ERRNO_CODES', '__setErrNo'],
  $FS__postset: 'FS.ignorePermissions = false;',
  $FS: {
    // The main file system tree. All the contents are inside this.
    root: {
      read: true,
      write: false,
      isFolder: true,
      isDevice: false,
      timestamp: new Date(),
      inodeNumber: 1,
      contents: {}
    },
    // The path to the current folder.
    currentPath: '/',
    // The inode to assign to the next created object.
    nextInode: 2,
    // The file creation mask used by the program.
    cmask: 0x1ff,  // S_IRWXU | S_IRWXG | S_IRWXO.
    // Currently opened file or directory streams. Padded with null so the zero
    // index is unused, as the indices are used as pointers. This is not split
    // into separate fileStreams and folderStreams lists because the pointers
    // must be interchangeable, e.g. when used in fdopen().
    streams: [null],
    // Whether we are currently ignoring permissions. Useful when preparing the
    // filesystem and creating files inside read-only folders.
    ignorePermissions: true,
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
            return FS.analyzePath([link].concat(path).join('/'),
                                  dontResolveLastLink, linksVisited + 1);
          }
          traversed.push(target);
          if (path.length == 0) {
            ret.exists = true;
            ret.path = traversed.join('/');
            ret.object = current;
          }
        }
        return ret;
      }
      return ret;
    },
    // Finds the file system object at a given path. If dontResolveLastLink is
    // set to true and the object is a symbolic link, it will be returned as is
    // instead of being resolved. Links embedded in the path as still resolved.
    findObject: function(path, dontResolveLastLink) {
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
        timestamp: new Date(),
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
        var dataArray = [];
        for (var i = 0; i < data.length; i++) dataArray.push(data.charCodeAt(i));
        data = dataArray;
      }
      var properties = {isDevice: false, contents: data};
      return FS.createFile(parent, name, properties, canRead, canWrite);
    },
    // Creates a file record for lazy-loading from a URL.
    createLazyFile: function(parent, name, url, canRead, canWrite) {
      var properties = {isDevice: false, url: url};
      return FS.createFile(parent, name, properties, canRead, canWrite);
    },
    // Creates a link to a sepcific local path.
    createLink: function(parent, name, target, canRead, canWrite) {
      var properties = {isDevice: false, link: target};
      return FS.createFile(parent, name, properties, canRead, canWrite);
    },
    // Creates a character device with input and output callbacks:
    //   input: Takes no parameters, returns a byte value or null if no data is
    //          currently available.
    //   output: Takes a byte value; doesn't return anything.
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
      if (obj.isDevice || obj.isFolder || obj.link ||
          'contents' in obj) return true;
      var success = true;
      if (typeof XMLHttpRequest !== 'undefined') {
        // Browser.
        // TODO: Use mozResponseArrayBuffer, responseStream, etc. if available.
        var xhr = new XMLHttpRequest();
        xhr.open('GET', obj.url, false);
        xhr.overrideMimeType('text/plain; charset=x-user-defined');  // Binary.
        xhr.send(null);
        if (xhr.status != 200 && xhr.status != 0) success = false;
        obj.contents = intArrayFromString(xhr.responseText || '');
      } else if (typeof read !== 'undefined') {
        // Command-line.
        try {
          obj.contents = intArrayFromString(read(obj.url));
        } catch (e) {
          success = false;
        }
      } else {
        throw new Error('Cannot load without read() or XMLHttpRequest.');
      }
      if (!success) ___setErrNo(ERRNO_CODES.EIO);
      return success;
    }
  },

  // ==========================================================================
  // dirent.h
  // ==========================================================================

  __dirent_struct_layout: Runtime.generateStructInfo(null, '%struct.dirent'),
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
    var id = FS.streams.length;
    var contents = [];
    for (var key in target.contents) contents.push(key);
    FS.streams[id] = {
      isFolder: true,
      path: path,
      object: target,
      // Remember the contents at the time of opening in an array, so we can
      // seek between them relying on a single order.
      contents: contents,
      // An index into contents. Special values: -2 is ".", -1 is "..".
      position: -2,
      // Each stream has its own area for readdir() returns.
      currentEntry: _malloc(___dirent_struct_layout.__size__)
    };
    return id;
  },
  closedir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  closedir: function(dirp) {
    // int closedir(DIR *dirp);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/closedir.html
    if (!FS.streams[dirp] || !FS.streams[dirp].isFolder) {
      return ___setErrNo(ERRNO_CODES.EBADF);
    } else {
      _free(FS.streams[dirp].currentEntry);
      delete FS.streams[dirp];
      return 0;
    }
  },
  telldir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  telldir: function(dirp) {
    // long int telldir(DIR *dirp);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/telldir.html
    if (!FS.streams[dirp] || !FS.streams[dirp].isFolder) {
      return ___setErrNo(ERRNO_CODES.EBADF);
    } else {
      return FS.streams[dirp].position;
    }
  },
  seekdir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  seekdir: function(dirp, loc) {
    // void seekdir(DIR *dirp, long int loc);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/seekdir.html
    if (!FS.streams[dirp] || !FS.streams[dirp].isFolder) {
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
    if (!FS.streams[dirp] || !FS.streams[dirp].isFolder) {
      return ___setErrNo(ERRNO_CODES.EBADF);
    }
    var stream = FS.streams[dirp];
    var loc = stream.position;
    var entries = 0;
    for (var key in stream.contents) entries++;
    if (loc < -2 || loc >= entries) {
      {{{ makeSetValue('result', '0', '0', 'i8*') }}}
    } else {
      var name, inode;
      if (loc === -2) {
        name = '.';
        inode = 1; // Really undefined.
      } else if (loc === -1) {
        name = '..';
        inode = 1; // Really undefined.
      } else {
        name = stream.contents[loc];
        inode = stream.object.contents[name].inodeNumber;
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
      var type = stream.isDevice ? 2 // DT_CHR, character device.
               : stream.isFolder ? 4 // DT_DIR, directory.
               : stream.link !== undefined ? 10 // DT_LNK, symbolic link.
               : 8; // DT_REG, regular file.
      {{{ makeSetValue('entry', 'offsets.d_type', 'type', 'i8') }}}
      {{{ makeSetValue('result', '0', 'entry', 'i8*') }}}
    }
    return 0;
  },
  readdir__deps: ['readdir_r', '__setErrNo', '$ERRNO_CODES'],
  readdir: function(dirp) {
    // struct dirent *readdir(DIR *dirp);
    // http://pubs.opengroup.org/onlinepubs/007908799/xsh/readdir_r.html
    if (!FS.streams[dirp] || !FS.streams[dirp].isFolder) {
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
  // TODO: Check if we need to link any aliases.

  // ==========================================================================
  // utime.h
  // ==========================================================================

  __utimbuf_struct_layout: Runtime.generateStructInfo(null, '%struct.utimbuf'),
  utime__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', '__utimbuf_struct_layout'],
  utime: function(path, times) {
    // int utime(const char *path, const struct utimbuf *times);
    // http://pubs.opengroup.org/onlinepubs/009695399/basedefs/utime.h.html
    var time;
    if (times) {
      // NOTE: We don't keep track of access timestamps.
      var offset = ___utimbuf_struct_layout.modtime;
      time = {{{ makeGetValue('times', 'offset', 'i32') }}}
      time = new Date(time * 1000);
    } else {
      time = new Date();
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
        me.ret = allocate(['.'.charCodeAt(0), 0], 'i8', ALLOC_STATIC);
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

  __stat_struct_layout: Runtime.generateStructInfo(null, '%struct.stat'),
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
    var time = Math.floor(obj.timestamp.getTime() / 1000);
    if (offsets.st_atime === undefined) {
      offsets.st_atime = offsets.st_atim.tv_sec;
      offsets.st_mtime = offsets.st_mtim.tv_sec;
      offsets.st_ctime = offsets.st_ctim.tv_sec;
      var nanosec = (obj.timestamp.getTime() % 1000) * 1000;
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
    {{{ makeSetValue('buf', 'offsets.st_dev', 'dev', 'i64') }}}
    {{{ makeSetValue('buf', 'offsets.st_rdev', 'rdev', 'i64') }}}
    // NOTE: These two may be i64, depending on compilation options.
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
      var pathPtr = allocate(pathArray, 'i8', ALLOC_NORMAL);
      var result = _stat(pathPtr, buf);
      _free(pathPtr);
      return result;
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
    obj.timestamp = new Date();
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
      var pathPtr = allocate(pathArray, 'i8', ALLOC_NORMAL);
      var result = _chmod(pathPtr, mode);
      _free(pathPtr);
      return result;
    }
  },
  // TODO: Test once cmask is used.
  umask__deps: ['$FS'],
  umask: function(newMask) {
    var oldMask = FS.cmask;
    FS.cmask = newMask;
    return oldMask;
  },
  __01fstat64_: 'fstat',
  __01stat64_: 'stat',
  __01lstat64_: 'lstat',
  // TODO: Check if other aliases are needed.

  // ==========================================================================
  // sys/statvfs.h
  // ==========================================================================

  __statvfs_struct_layout: Runtime.generateStructInfo(null, '%struct.statvfs'),
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

  // ==========================================================================
  // fcntl.h
  // ==========================================================================

  __flock_struct_layout: Runtime.generateStructInfo(null, '%struct.flock'),
  open__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  open: function(path, oflag, mode) {
    // int open(const char *path, int oflag, ...);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/open.html
    // NOTE: This implementation tries to mimic glibc rather that strictly
    // following the POSIX standard.

    // Simplify flags.
    var accessMode = oflag & 0x3;  // O_ACCMODE.
    var isWrite = accessMode != 0x0;  // O_RDONLY.
    var isRead = accessMode != 0x1;  // O_WRONLY.
    var isCreate = Boolean(oflag & 0x40);  // O_CREAT.
    var isExistCheck = Boolean(oflag & 0x80);  // O_EXCL.
    var isTruncate = Boolean(oflag & 0x200);  // O_TRUNC.
    var isAppend = Boolean(oflag & 0x400);  // O_APPEND.

    // Verify path.
    path = FS.analyzePath(Pointer_stringify(path));
    if (!path.parentExists) {
      ___setErrNo(path.error);
      return -1;
    }
    var target = path.object || null;

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
    }

    // Actually create an open stream.
    var id = FS.streams.length;
    FS.streams[id] = {
      isFolder: false,
      path: path.path,
      object: target,
      position: 0,
      isRead: isRead,
      isWrite: isWrite,
      isAppend: isAppend
    };
    return id;
  },
  creat__deps: ['open'],
  creat: function(path, mode) {
    // int creat(const char *path, mode_t mode);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/creat.html
    return _open(path, 0x241, mode);  // O_WRONLY | O_CREAT | O_TRUNC.
  },
  fcntl__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', '__flock_struct_layout'],
  fcntl: function(fildes, cmd, arg) {
    // int fcntl(int fildes, int cmd, ...);
    // http://pubs.opengroup.org/onlinepubs/009695399/functions/fcntl.html
    if (!(fildes in FS.streams)) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
    var stream = FS.streams[fildes];
    switch (cmd) {
      case 0:  // F_DUPFD.
        if (arg < 0) {
          ___setErrNo(ERRNO_CODES.EINVAL);
          return -1;
        }
        var newStream = {};
        for (var member in stream) {
          newStream[member] = stream[member];
        }
        if (arg in FS.streams) arg = FS.streams.length;
        FS.streams[arg] = newStream;
        return arg;
      case 1:  // F_GETFD.
      case 2:  // F_SETFD.
        return 0;  // FD_CLOEXEC makes no sense for a single process.
      case 3:  // F_GETFL.
        var flags = 0;
        if (stream.isRead && stream.isWrite) flags = 0x2;  // O_RDWR.
        else if (!stream.isRead && stream.isWrite) flags = 0x1;  // O_WRONLY.
        else if (stream.isRead && !stream.isWrite) flags = 0x0;  // O_RDONLY.
        if (stream.isAppend) flags |= 0x400;  // O_APPEND.
        // Synchronization and blocking flags are irrelevant to us.
        return flags;
      case 4:  // F_SETFL.
        stream.isAppend = Boolean(arg | 0x400);  // O_APPEND.
        // Synchronization and blocking flags are irrelevant to us.
        return 0;
      case 5:  // F_GETLK.
        var offset = ___flock_struct_layout.l_type;
        // We're always unlocked.
        {{{ makeSetValue('arg', 'offset', '2', 'i16') }}}  // F_UNLCK.
        return 0;
      case 6:  // F_SETLK.
      case 7:  // F_SETLKW.
        // Pretend that the locking is successful.
        return 0;
      case 8:  // F_SETOWN.
      case 9:  // F_GETOWN.
        // These are for sockets. We don't have them implemented (yet?).
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
    if (!(fd in FS.streams) || FS.streams[fd].link ||
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

  __pollfd_struct_layout: Runtime.generateStructInfo(null, '%struct.pollfd'),
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
      if (fd in FS.streams) {
        var stream = FS.streams[fd];
        if (events & 0x1) revents |= 0x1;  // POLLIN.
        if (events & 0x4) revents |= 0x4;  // POLLOUT.
      } else {
        if (events & 0x20) revents |= 0x20;  // POLLNVAL.
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
    path = FS.absolutePath(path);
    // TODO: Resolve path so that no element of it is a link.
    var target = FS.findObject(path);
    if (target === null) return -1;
    if (!target.isFolder) {
      ___setErrNo(ERRNO_CODES.ENOTDIR);
      return -1;
    } else {
      FS.currentPath = path;
      return 0;
    }
  },
  chown__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  chown: function(path, owner, group, dontResolveLastLink) {
    // int chown(const char *path, uid_t owner, gid_t group);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/chown.html
    // We don't support multiple users, so changing ownership makes no sense.
    // NOTE: The path argument may be a string, to simplify fchown().
    // NOTE: dontResolveLastLink is a shortcut for lstat(). It should never be
    //       used in client code.
    if (typeof path !== 'string') path = Pointer_stringify(path);
    var target = FS.findObject(path, dontResolveLastLink);
    if (target === null) return -1;
    target.timestamp = new Date();
    return 0;
  },
  close__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  close: function(fildes) {
    // int close(int fildes);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/close.html
    if (FS.streams[fildes]) {
      delete FS.streams[fildes];
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
    return _fcntl(fildes, 0, 0);  // F_DUPFD.
  },
  dup2__deps: ['$FS', '__setErrNo', '$ERRNO_CODES', 'fcntl'],
  dup2: function(fildes, fildes2) {
    // int dup2(int fildes, int fildes2);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/dup.html
    // TODO: Verify: Duplicate FD to FD2, closing FD2 and making it open on the same file.
    if (fildes2 < 0) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    } else if (fildes === fildes2 && FS.streams[fildes]) {
      return fildes;
    } else {
      return _fcntl(fildes, 0, fildes2);  // F_DUPFD.
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
    // TODO: Implement (compile from source?).
    ___setErrNo(ERRNO_CODES.ENOSYS);
    return 0;
  },
  encrypt: function(key, salt) {
    // void encrypt(char block[64], int edflag);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/encrypt.html
    // TODO: Implement (compile from source?).
    ___setErrNo(ERRNO_CODES.ENOSYS);
  },
  fpathconf__deps: ['__setErrNo', '$ERRNO_CODES'],
  fpathconf: function(fildes, name) {
    // long fpathconf(int fildes, int name);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/encrypt.html
    // NOTE: The first parameter is ignored, so pathconf == fpathconf.
    // The constants here aren't real values. Just mimicing glibc.
    switch (name) {
      case 0:  // _PC_LINK_MAX.
        return 32000;
      case 1:  // _PC_MAX_CANON.
      case 2:  // _PC_MAX_INPUT.
      case 3:  // _PC_NAME_MAX.
        return 255;
      case 4:  // _PC_PATH_MAX.
      case 5:  // _PC_PIPE_BUF.
      case 16:  // _PC_REC_MIN_XFER_SIZE.
      case 17:  // _PC_REC_XFER_ALIGN.
      case 18:  // _PC_ALLOC_SIZE_MIN.
        return 4096;
      case 6:  // _PC_CHOWN_RESTRICTED.
      case 7:  // _PC_NO_TRUNC.
      case 20:  // _PC_2_SYMLINKS.
        return 1;
      case 8:  // _PC_VDISABLE.
        return 0;
      case 9:  // _PC_SYNC_IO.
      case 10:  // _PC_ASYNC_IO.
      case 11:  // _PC_PRIO_IO.
      case 12:  // _PC_SOCK_MAXBUF.
      case 14:  // _PC_REC_INCR_XFER_SIZE.
      case 15:  // _PC_REC_MAX_XFER_SIZE.
      case 19:  // _PC_SYMLINK_MAX.
        return -1;
      case 13:  // _PC_FILESIZEBITS.
        return 64;
      defult:
        ___setErrNo(ERRNO_CODES.EINVAL);
        return -1;
    }
    // Should never be reached. Only to silence strict warnings.
    return -1;
  },
  pathconf: 'fpathconf',
  fsync__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  fsync: function(fildes, owner, group) {
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
      } else {
        var contents = target.contents;
        if (length < contents.length) contents.length = length;
        else while (length > contents.length) contents.push(0);
        target.timestamp = new Date();
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
        {{{ makeSetValue('buf', 'i', 'FS.currentPath[i]', 'i8') }}}
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
    if (FS.streams[fildes]) {
      var object = FS.streams[fildes].object;
      if (object.isDevice && object.input && object.output) {
        // As far as we're concerned, a TTY is any device which supports both
        // input and output.
        return 0;
      } else {
        ___setErrNo(ERRNO_CODES.ENOTTY);
        return -1;
      }
    } else {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    }
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
      // Pretend whatever locking or unlocking operation succeeded. Lokcing does
      // not make sense since we have a single process/thread.
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
    if (FS.streams[fildes] && !FS.streams[fildes].isDevice) {
      var stream = FS.streams[fildes];
      var position = offset;
      if (whence === 1) {  // SEEK_CUR.
        position += stream.position;
      } else if (whence === 2) {  // SEEK_END.
        position += stream.contents.length;
      }
      stream.position = position;
      return position;
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
    if (!stream || stream.isDevice) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    } else if (!stream.isRead) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else if (stream.isFolder) {
      ___setErrNo(ERRNO_CODES.EISDIR);
      return -1;
    } else if (nbyte < 0 || offset < 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      var contents = stream.object.contents;
      var size = Math.min(contents.length - offset, nbyte);
      for (var i = 0; i < size; i++) {
        {{{ makeSetValue('buf', 'i', 'contents[offset + i]', 'i8') }}}
      }
      return i;
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
      if (stream.isDevice) {
        if (stream.input) {
          for (var i = 0; i < nbyte; i++) {
            try {
              var result = stream.input();
            } catch (e) {
              ___setErrNo(ERRNO_CODES.EIO);
              return -1;
            }
            if (result === null || result === undefined) break;
            {{{ makeSetValue('buf', 'i', 'result', 'i8') }}}
          }
          return i;
        } else {
          ___setErrNo(ERRNO_CODES.ENXIO);
          return -1;
        }
      } else {
        var bytesRead = _pread(fildes, buf, nbyte, stream.position);
        if (bytesRead != -1) stream.position += bytesRead;
        return bytesRead;
      }
    }
  },
  sync: function() {
    // void sync(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/sync.html
    // All our writing is already synchronized. This is a no-op.
  },
  rmdir__deps: ['$FS', '__setErrNo', '$ERRNO_CODES',
                'dirname', 'basename', 'strcpy', 'strlen'],
  rmdir: function(path) {
    // int rmdir(const char *path);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/rmdir.html
    var buffer = _malloc(_strlen(path) + 1);
    var parent = Pointer_stringify(_dirname(_strcpy(buffer, path)));
    var name = Pointer_stringify(_basename(_strcpy(buffer, path)));
    var absolutePath = FS.absolutePath(Pointer_stringify(path));
    _free(buffer);
    parent = FS.findObject(parent);
    if (parent === null) return -1;
    if (!parent.read) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else if (!parent.contents.hasOwnProperty(name)) {
      ___setErrNo(ERRNO_CODES.ENOENT);
      return -1;
    } else if (!parent.contents[name].isFolder) {
      ___setErrNo(ERRNO_CODES.ENOTDIR);
      return -1;
    } else if (!parent.contents[name].write) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else if (!parent.contents[name].contents.length != 0) {
      ___setErrNo(ERRNO_CODES.ENOTEMPTY);
      return -1;
    } else if (absolutePath == FS.currentPath) {
      ___setErrNo(ERRNO_CODES.EBUSY);
      return -1;
    } else {
      delete parent.contents[name];
      return 0;
    }
  },
  unlink__deps: ['$FS', '__setErrNo', '$ERRNO_CODES',
                 'dirname', 'basename', 'strcpy', 'strlen'],
  unlink: function(path) {
    // int unlink(const char *path);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/unlink.html
    var buffer = _malloc(_strlen(path) + 1);
    var parent = Pointer_stringify(_dirname(_strcpy(buffer, path)));
    var name = Pointer_stringify(_basename(_strcpy(buffer, path)));
    _free(buffer);
    parent = FS.findObject(parent);
    if (parent === null) return -1;
    if (!parent.read) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else if (!parent.contents.hasOwnProperty(name)) {
      ___setErrNo(ERRNO_CODES.ENOENT);
      return -1;
    } else if (parent.contents[name].isFolder) {
      ___setErrNo(ERRNO_CODES.EISDIR);
      return -1;
    } else if (!parent.contents[name].write) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else {
      delete parent.contents[name];
      return 0;
    }
  },
  ttyname__deps: ['ttyname_r'],
  ttyname: function(fildes) {
    // char *ttyname(int fildes);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/ttyname.html
    if (!_ttyname.ret) _ttyname.ret = _malloc(256);
    return _ttyname_r(fildes, _ttyname.ret, 256);
  },
  ttyname_r__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  ttyname_r: function(fildes, name, namesize) {
    // int ttyname_r(int fildes, char *name, size_t namesize);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/ttyname.html
    var stream = FS.streams[fildes];
    if (!stream) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return 0;
    } else if (!stream.isDevice || !stream.input || !stream.output) {
      ___setErrNo(ERRNO_CODES.ENOTTY);
      return 0;
    } else {
      var ret = stream.path;
      if (namesize < ret.length + 1) {
        ___setErrNo(ERRNO_CODES.ERANGE);
        return 0;
      } else {
        for (var i = 0; i < ret.length; i++) {
          {{{ makeSetValue('name', 'i', 'ret.charCodeAt(i)', 'i8') }}}
        }
        {{{ makeSetValue('name', 'i', '0', 'i8') }}}
        return name;
      }
    }
  },
  symlink__deps: ['$FS', 'mknod'],
  symlink: function(path1, path2) {
    // int symlink(const char *path1, const char *path2);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/symlink.html
    var result = _mknod(path2, 0x10000 | 0x1C0, 0);  // S_IFREG, S_IRUSR | S_IWUSR | S_IXUSR.
    if (result == 0) {
      var target = FS.findObject(Pointer_stringify(path2));
      delete target.contents;
      target.link = Pointer_stringify(path1);
      return 0;
    } else {
      return result;
    }
  },
  readlink__deps: ['$FS', '__setErrNo', '$ERRNO_CODES'],
  readlink: function(path, buf, bufsize) {
    // ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/readlink.html
    var target = FS.findObject(Pointer_stringify(path), true);
    if (target === null) return -1;
    if (target.link !== undefined) {
      var length = Math.min(bufsize, target.link.length);
      for (var i = 0; i < length; i++) {
        {{{ makeSetValue('name', 'i', 'target.link.charCodeAt(i)', 'i8') }}}
      }
      if (length > bufsize) {{{ makeSetValue('name', 'i++', '0', 'i8') }}}
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
    if (!stream || stream.isDevice) {
      ___setErrNo(ERRNO_CODES.EBADF);
      return -1;
    } else if (!stream.isWrite) {
      ___setErrNo(ERRNO_CODES.EACCES);
      return -1;
    } else if (stream.isFolder) {
      ___setErrNo(ERRNO_CODES.EISDIR);
      return -1;
    } else if (nbyte < 0 || offset < 0) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      var contents = stream.object.contents;
      while (contents.length < offset) contents.push(0);
      for (var i = 0; i < nbyte; i++) {
        contents[offset + i] = {{{ makeGetValue('buf', 'i', 'i8') }}};
      }
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
      if (stream.isDevice) {
        if (stream.output) {
          for (var i = 0; i < nbyte; i++) {
            try {
              stream.output({{{ makeGetValue('buf', 'i', 'i8') }}});
            } catch (e) {
              ___setErrNo(ERRNO_CODES.EIO);
              return -1;
            }
          }
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
  confstr__deps: ['__setErrNo', '$ERRNO_CODES'],
  confstr: function(name, buf, len) {
    // size_t confstr(int name, char *buf, size_t len);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/confstr.html
    var value;
    switch (name) {
      case 0:  // _CS_PATH.
        // TODO: Get from environment variable.
        value = '/';
        break;
      case 1:  // _CS_POSIX_V6_WIDTH_RESTRICTED_ENVS.
        // Mimicing glibc.
        value = 'POSIX_V6_ILP32_OFF32\nPOSIX_V6_ILP32_OFFBIG';
        break;
      case 2:  // _CS_GNU_LIBC_VERSION.
        // This JS implementation was tested against this glibc version.
        value = 'glibc 2.14';
        break;
      case 3:  // _CS_GNU_LIBPTHREAD_VERSION.
        // We don't support pthread.
        value = '';
        break;
      case 1118:  // _CS_POSIX_V6_ILP32_OFF32_LIBS.
      case 1122:  // _CS_POSIX_V6_ILP32_OFFBIG_LIBS.
      case 1124:  // _CS_POSIX_V6_LP64_OFF64_CFLAGS.
      case 1125:  // _CS_POSIX_V6_LP64_OFF64_LDFLAGS.
      case 1126:  // _CS_POSIX_V6_LP64_OFF64_LIBS.
      case 1128:  // _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS.
      case 1129:  // _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS.
      case 1130:  // _CS_POSIX_V6_LPBIG_OFFBIG_LIBS.
        value = '';
        break;
      case 1116:  // _CS_POSIX_V6_ILP32_OFF32_CFLAGS.
      case 1117:  // _CS_POSIX_V6_ILP32_OFF32_LDFLAGS.
      case 1121:  // _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS.
        value = '-m32';
        break;
      case 1120:  // _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS.
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
      if (length > len) {{{ makeSetValue('buf', 'i++', '0', 'i8') }}}
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
    __shutdownRuntime__();
    ABORT = true;
    throw 'exit(' + status + ') called, at ' + new Error().stack;
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
  getgroups__deps: ['__setErrNo', '$ERRNO_CODES'],
  getgroups: function(gidsetsize, grouplist) {
    // int getgroups(int gidsetsize, gid_t grouplist[]);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getgroups.html
    if (gidsetsize < 1) {
      ___setErrNo(ERRNO_CODES.EINVAL);
      return -1;
    } else {
      {{{ makeSetValue('grouplist', '0', '0', 'i8') }}}
      return 1;
    }
  },
  gethostid: function() {
    // long gethostid(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/gethostid.html
    return 42;
  },
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
    if (length > namelen) {{{ makeSetValue('name', 'i', '0', 'i8') }}}
    return 0;
  },
  getlogin: ['getlogin_r'],
  getlogin: function() {
    // char *getlogin(void);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getlogin.html
    if (!_getlogin.ret) _getlogin.ret = _malloc(8);
    return _getlogin_r(_getlogin.ret, 8);
  },
  getlogin_r__deps: ['__setErrNo', '$ERRNO_CODES'],
  getlogin_r: function(name, namesize) {
    // int getlogin_r(char *name, size_t namesize);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/getlogin.html
    var ret = 'root';
    for (var i = 0; i < ret.length; i++) {
      {{{ makeSetValue('name', 'i', 'ret.charCodeAt(i)', 'i8') }}}
    }
    {{{ makeSetValue('name', 'i', '0', 'i8') }}}
    return name;
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
  setpgid__deps: ['usleep'],
  sleep: function(seconds) {
    // unsigned sleep(unsigned seconds);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/sleep.html
    return _usleep(seconds * 1e6);
  },
  usleep: function(useconds) {
    // int usleep(useconds_t useconds);
    // http://pubs.opengroup.org/onlinepubs/000095399/functions/usleep.html
    // We're single-threaded, so use a busy loop. Super-ugly.
    var start = new Date().getTime();
    while (new Date().getTime() - start < useconds);
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
  sysconf: function(name_) {
    // TODO: Implement.
    // XXX we only handle _SC_PAGE_SIZE/PAGESIZE for now, 30 on linux, 29 on OS X... be careful here!
    switch(name_) {
      case 29: case 30: return PAGE_SIZE;
      case 2: return 1000000; // _SC_CLK_TCK
      default: throw 'unknown sysconf param: ' + name_;
    }
  },
  // TODO: Check if these aliases are correct and if any others are needed.
  __01open64_: 'open',
  __01lseek64_: 'lseek',

  // ==========================================================================

  _scanString: function() {
    // Supports %x, %4x, %d.%d, %s
    var str = Pointer_stringify(arguments[0]);
    var stri = 0;
    var fmt = Pointer_stringify(arguments[1]);
    var fmti = 0;
    var args = Array.prototype.slice.call(arguments, 2);
    var argsi = 0;
    var fields = 0;
    while (fmti < fmt.length) {
      if (fmt[fmti] === '%') {
        fmti++;
        var max_ = parseInt(fmt[fmti]);
        if (!isNaN(max_)) fmti++;
        var type = fmt[fmti];
        fmti++;
        var curr = 0;
        while ((curr < max_ || isNaN(max_)) && stri+curr < str.length) {
          if ((type === 'd' && parseInt(str[stri+curr]) >= 0) ||
              (type === 'x' && parseInt(str[stri+curr].replace(/[a-fA-F]/, 5)) >= 0) ||
              (type === 's')) {
            curr++;
          } else {
            break;
          }
        }
        if (curr === 0) return 0; // failure
        var text = str.substr(stri, curr);
        stri += curr;
        switch (type) {
          case 'd': {
            {{{ makeSetValue('args[argsi]', '0', 'parseInt(text)', 'i32') }}}
            break;
          }
          case 'x': {
            {{{ makeSetValue('args[argsi]', '0', 'eval("0x" + text)', 'i32') }}}
            break;
          }
          case 's': {
            var array = intArrayFromString(text);
            for (var j = 0; j < array.length; j++) {
              {{{ makeSetValue('args[argsi]', 'j', 'array[j]', 'i8') }}}
            }
            break;
          }
        }
        argsi++;
        fields++;
      } else { // not '%'
        if (fmt[fmti] === str[stri]) {
          fmti++;
          stri++;
        } else {
          break;
        }
      }
    }
    return { fields: fields, bytes: stri };
  },
  sscanf__deps: ['_scanString'],
  sscanf: function() {
    return __scanString.apply(null, arguments).fields;
  },

  _formatString__deps: ['$STDIO', 'isdigit'],
  _formatString: function() {
    var cStyle = false;
    var textIndex = arguments[0];
    var argIndex = 1;
    if (textIndex < 0) {
      cStyle = true;
      textIndex = -textIndex;
      argIndex = arguments[1];
    } else {
      var _arguments = arguments;
    }
    function getNextArg(isFloat, size) {
      var ret;
      if (!cStyle) {
        ret = _arguments[argIndex];
        argIndex++;
      } else {
        if (isFloat) {
          ret = {{{ makeGetValue(0, 'argIndex', 'double') }}};
        } else {
          ret = {{{ makeGetValue(0, 'argIndex', 'i32') }}};
        }
        argIndex += {{{ QUANTUM_SIZE === 1 ? 1 : 'Math.max(4, size || 0)' }}};
      }
      return +ret; // +: boolean=>int
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
          width = getNextArg();
          textIndex++;
          next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
        } else {
          while (_isdigit(next)) {
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
            precision = getNextArg();
            textIndex++;
          } else {
            while(1) {
              var precisionChr = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};
              if (!_isdigit(precisionChr)) break;
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
              argSize = 1; // char
            } else {
              argSize = 2; // short
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
            argSize = undefined;
        }
        if (argSize !== undefined) textIndex++;
        next = {{{ makeGetValue(0, 'textIndex+1', 'i8') }}};

        // Handle type specifier.
        if (['d', 'i', 'u', 'o', 'x', 'X', 'p'].indexOf(String.fromCharCode(next)) != -1) {
          // Integer.
          var signed = next == 'd'.charCodeAt(0) || next == 'i'.charCodeAt(0);
          var currArg = getNextArg(false, argSize);
          // Truncate to requested size.
          argSize = argSize || 4;
          if (argSize <= 4) {
            var limit = Math.pow(256, argSize) - 1;
            currArg = (signed ? reSign : unSign)(currArg & limit, argSize * 8);
          }
          // Format the number.
          var currAbsArg = Math.abs(currArg);
          var argText;
          var prefix = '';
          if (next == 'd'.charCodeAt(0) || next == 'i'.charCodeAt(0)) {
            argText = currAbsArg.toString(10);
          } else if (next == 'u'.charCodeAt(0)) {
            argText = unSign(currArg, 8 * argSize).toString(10);
            currArg = Math.abs(currArg);
          } else if (next == 'o'.charCodeAt(0)) {
            argText = (flagAlternative ? '0' : '') + currAbsArg.toString(8);
          } else if (next == 'x'.charCodeAt(0) || next == 'X'.charCodeAt(0)) {
            prefix = flagAlternative ? '0x' : '';
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
            prefix = '0x';
            argText = currAbsArg.toString(16);
          }
          if (precisionSet) {
            while (argText.length < precision) {
              argText = '0' + argText;
            }
          }

          // Add sign.
          if (currArg < 0) {
            prefix = '-' + prefix;
          } else if (flagAlwaysSigned) {
            prefix = '+' + prefix;
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
          var currArg = getNextArg(true, argSize);
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
          var copiedString = String_copy(getNextArg());
          if (precisionSet && copiedString.length > precision) {
            copiedString = copiedString.slice(0, precision);
          }
          if (!flagLeftAlign) {
            while (copiedString.length < width--) {
              ret.push(' '.charCodeAt(0));
            }
          }
          ret = ret.concat(copiedString);
          if (flagLeftAlign) {
            while (copiedString.length < width--) {
              ret.push(' '.charCodeAt(0));
            }
          }
        } else if (next == 'c'.charCodeAt(0)) {
          // Character.
          if (flagLeftAlign) ret.push(getNextArg());
          while (--width > 0) {
            ret.push(' '.charCodeAt(0));
          }
          if (!flagLeftAlign) ret.push(getNextArg());
        } else if (next == 'n'.charCodeAt(0)) {
          // Write the length written so far to the next parameter.
          {{{ makeSetValue('getNextArg()', '0', 'ret.length', 'i32') }}}
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
    return allocate(ret.concat(0), 'i8', ALLOC_STACK); // NB: Stored on the stack
    //var len = ret.length+1;
    //var ret = allocate(ret.concat(0), 0, ALLOC_STACK); // NB: Stored on the stack
    //STACKTOP -= len; // XXX horrible hack. we rewind the stack, to 'undo' the alloc we just did.
    //                 // the point is that this works if nothing else allocs on the stack before
    //                 // the string is read, which should be true - it is very transient, see the *printf* functions below.
    //return ret;
  },

  printf__deps: ['_formatString'],
  printf: function() {
    __print__(Pointer_stringify(__formatString.apply(null, arguments)));
  },

  sprintf__deps: ['strcpy', '_formatString'],
  sprintf: function() {
    var str = arguments[0];
    var args = Array.prototype.slice.call(arguments, 1);
    _strcpy(str, __formatString.apply(null, args)); // not terribly efficient
  },

  snprintf__deps: ['strncpy', '_formatString'],
  snprintf: function() {
    var str = arguments[0];
    var num = arguments[1];
    var args = Array.prototype.slice.call(arguments, 2);
    _strncpy(str, __formatString.apply(null, args), num); // not terribly efficient
  },

  puts: function(p) {
    __print__(Pointer_stringify(p) + '\n');
  },

  putc: 'fputc',
  _IO_putc: 'fputc',

  putchar: function(p) {
    __print__(String.fromCharCode(p));
  },
  _ZNSo3putEc: 'putchar',

  _ZNSo5flushEv: function() {
    __print__('\n');
  },

  vsprintf__deps: ['strcpy', '_formatString'],
  vsprintf: function(dst, src, ptr) {
    _strcpy(dst, __formatString(-src, ptr));
  },

  vsnprintf__deps: ['_formatString'],
  vsnprintf: function(dst, num, src, ptr) {
    var text = __formatString(-src, ptr); // |-|src tells formatstring to use C-style params (typically they are from varargs)
    var i;
    for (i = 0; i < num; i++) {
      {{{ makeCopyValues('dst+i', 'text+i', 1, 'i8') }}}
      if ({{{ makeGetValue('dst', 'i', 'i8') }}} == 0) break;
    }
    return i; // Actually, should return how many *would* have been written, if the |num| had not stopped us.
  },

  fileno: function(file) {
    return file;
  },

  clearerr: function(stream) {
  },

  flockfile: function(file) {
  },

  funlockfile: function(file) {
  },

  stdin: 0,
  stdout: 0,
  stderr: 0,

  $STDIO__postset: 'STDIO.init()',
  $STDIO__deps: ['stdin', 'stdout', 'stderr'],
  $STDIO: {
    streams: {},
    filenames: {},
    counter: 1,
    SEEK_SET: 0, /* Beginning of file.  */
    SEEK_CUR: 1, /* Current position.   */
    SEEK_END: 2, /* End of file.        */
    init: function() {
      _stdin = allocate([0], 'void*', ALLOC_STATIC);
      {{{ makeSetValue('_stdin', '0', "STDIO.prepare('<<stdin>>', null, null, true)", 'i32') }}};
      if (Module.stdin) {
        // Make sure stdin returns a newline
        var orig = Module.stdin;
        Module.stdin = function stdinFixed(prompt) {
          var ret = orig(prompt);
          if (ret[ret.length-1] !== '\n') ret = ret + '\n';
          return ret;
        }
      } else {
        Module.stdin = function stdin(prompt) {
          return window.prompt(prompt) || '';
        };
      }

      _stdout = allocate([0], 'void*', ALLOC_STATIC);
      {{{ makeSetValue('_stdout', '0', "STDIO.prepare('<<stdout>>', null, true)", 'i32') }}};

      _stderr = allocate([0], 'void*', ALLOC_STATIC);
      {{{ makeSetValue('_stderr', '0', "STDIO.prepare('<<stderr>>', null, true)", 'i32') }}};
    },
    cleanFilename: function(filename) {
      return filename.replace('./', '');
    },
    prepare: function(filename, data, print_, interactiveInput) {
      filename = STDIO.cleanFilename(filename);
      var stream = STDIO.counter++;
      STDIO.streams[stream] = {
        filename: filename,
        data: data ? data : [],
        position: 0,
        eof: 0,
        error: 0,
        interactiveInput: interactiveInput, // true for stdin - on the web, we allow interactive input
        print: print_ // true for stdout and stderr - we print when receiving data for them
      };
      STDIO.filenames[filename] = stream;
      return stream;
    },
    open: function(filename) {
      filename = STDIO.cleanFilename(filename);
      var stream = STDIO.filenames[filename];
      if (!stream) {
        // Not already cached; try to load it right now
        try {
          return STDIO.prepare(filename, readBinary(filename));
        } catch(e) {
          return 0;
        }
      }
      var info = STDIO.streams[stream];
      info.position = info.error = info.eof = 0;
      return stream;
    },
    read: function(stream, ptr, size) {
      var info = STDIO.streams[stream];
      if (!info) return -1;
      if (info.interactiveInput) {
        for (var i = 0; i < size; i++) {
          if (info.data.length === 0) {
            info.data = intArrayFromString(Module.stdin(PRINTBUFFER.length > 0 ? PRINTBUFFER : '?')).map(function(x) { return x === 0 ? 10 : x }); // change 0 to newline
            PRINTBUFFER = '';
            if (info.data.length === 0) return i;
          }
          {{{ makeSetValue('ptr', '0', 'info.data.shift()', 'i8') }}}
          ptr++;
        }
        return size;
      }
      for (var i = 0; i < size; i++) {
        if (info.position >= info.data.length) {
          info.eof = 1;
          return 0; // EOF
        }
        {{{ makeSetValue('ptr', '0', 'info.data[info.position]', 'i8') }}}
        info.position++;
        ptr++;
      }
      return size;
    },
    write: function(stream, ptr, size) {
      var info = STDIO.streams[stream];
      if (!info) return -1;
      if (info.print) {
        __print__(intArrayToString(Array_copy(ptr, size)));
      } else {
        for (var i = 0; i < size; i++) {
          info.data[info.position] = {{{ makeGetValue('ptr', '0', 'i8') }}};
          info.position++;
          ptr++;
        }
      }
      return size;
    }
  },

  fopen__deps: ['$STDIO'],
  fopen: function(filename, mode) {
    filename = Pointer_stringify(filename);
    mode = Pointer_stringify(mode);
    if (mode.indexOf('r') >= 0) {
      return STDIO.open(filename);
    } else if (mode.indexOf('w') >= 0) {
      return STDIO.prepare(filename);
    } else {
      return assert(false, 'fopen with odd params: ' + mode);
    }
  },
  __01fopen64_: 'fopen',

  fdopen: function(descriptor, mode) {
    // TODO: Check whether mode is acceptable for the current stream.
    return descriptor;
  },

  rewind__deps: ['$STDIO'],
  rewind: function(stream) {
    var info = STDIO.streams[stream];
    info.position = 0;
    info.error = 0;
  },

  fseek__deps: ['$STDIO'],
  fseek: function(stream, offset, whence) {
    var info = STDIO.streams[stream];
    if (whence === STDIO.SEEK_CUR) {
      offset += info.position;
    } else if (whence === STDIO.SEEK_END) {
      offset += info.data.length;
    }
    info.position = offset;
    info.eof = 0;
    return 0;
  },
  __01fseeko64_: 'fseek',

  ftell__deps: ['$STDIO'],
  ftell: function(stream) {
    return STDIO.streams[stream].position;
  },
  __01ftello64_: 'ftell',

  fread__deps: ['$STDIO'],
  fread: function(ptr, size, count, stream) {
    var info = STDIO.streams[stream];
    if (info.interactiveInput) return STDIO.read(stream, ptr, size*count);
    for (var i = 0; i < count; i++) {
      if (info.position + size > info.data.length) {
        info.eof = 1;
        return i;
      }
      STDIO.read(stream, ptr, size);
      ptr += size;
    }
    return count;
  },

  fwrite__deps: ['$STDIO'],
  fwrite: function(ptr, size, count, stream) {
    STDIO.write(stream, ptr, size*count);
    return count;
  },

  fclose__deps: ['$STDIO'],
  fclose: function(stream) {
    return 0;
  },

  feof__deps: ['$STDIO'],
  feof: function(stream) {
    return STDIO.streams[stream].eof;
  },

  ferror__deps: ['$STDIO'],
  ferror: function(stream) {
    return STDIO.streams[stream].error;
  },

  fprintf__deps: ['_formatString', '$STDIO'],
  fprintf: function() {
    var stream = arguments[0];
    var args = Array.prototype.slice.call(arguments, 1);
    var ptr = __formatString.apply(null, args);
    STDIO.write(stream, ptr, String_len(ptr));
  },

  vfprintf__deps: ['$STDIO', '_formatString'],
  vfprintf: function(stream, format, args) {
    var ptr = __formatString(-format, args);
    STDIO.write(stream, ptr, String_len(ptr));
  },

  fflush__deps: ['$STDIO'],
  fflush: function(stream) {
    var info = STDIO.streams[stream];
    if (info && info.print) {
      __print__(null);
    }
  },

  fputs__deps: ['$STDIO', 'fputc'],
  fputs: function(p, stream) {
    STDIO.write(stream, p, String_len(p));
  },

  fputc__deps: ['$STDIO'],
  fputc: function(chr, stream) {
    if (!Module._fputc_ptr) Module._fputc_ptr = _malloc(1);
    {{{ makeSetValue('Module._fputc_ptr', '0', 'chr', 'i8') }}}
    var ret = STDIO.write(stream, Module._fputc_ptr, 1);
    return (ret == -1) ? -1 /* EOF */ : chr;
  },

  getc__deps: ['$STDIO'],
  getc: function(file) {
    if (!Module._getc_ptr) Module._getc_ptr = _malloc(1);
    var ret = STDIO.read(file, Module._getc_ptr, 1);
    if (ret === 0) return -1; // EOF
    return {{{ makeGetValue('Module._getc_ptr', '0', 'i8') }}}
  },
  getc_unlocked: 'getc',
  _IO_getc: 'getc',

  getchar__deps: ['getc'],
  getchar: function() {
    return _getc(_stdin);
  },

  ungetc: function(chr, stream) {
    var f = STDIO.streams[stream];
    if (!f)
      return -1; // EOF
    if (!f.interactiveInput)
      f.position--;
    return chr;
  },

  gets: function(ptr) {
    var num = 0;
    while (STDIO.read({{{ makeGetValue('_stdin', '0', 'void*') }}}, ptr+num, 1) &&
           {{{ makeGetValue('ptr', 'num', 'i8') }}} !== 10) { num++; }
    if (num === 0) return 0;
    {{{ makeSetValue('ptr', 'num', 0, 'i8') }}}
    return ptr;
  },

  fscanf__deps: ['_scanString'],
  fscanf: function(stream, format) {
    var f = STDIO.streams[stream];
    if (!f)
      return -1; // EOF
    arguments[0] = allocate(f.data.slice(f.position).concat(0), 'i8', ALLOC_STACK);
    var ret = __scanString.apply(null, arguments);
    f.position += ret.bytes;
    return ret.fields;
  },

  // unix file IO, see http://rabbit.eng.miami.edu/info/functions/unixio.html

  mmap: function(start, num, prot, flags, stream, offset) {
    // Leaky and non-shared... FIXME
    var info = STDIO.streams[stream];
    if (!info) return -1;
    return allocate(info.data.slice(offset, offset+num), 'i8', ALLOC_NORMAL);
  },

  munmap: function(start, num) {
    _free(start); // FIXME: not really correct at all
  },

  setbuf: function(stream, buffer) {
    // just a stub
    assert(!buffer);
  },

  setvbuf: 'setbuf',

  // ==========================================================================
  // stdlib.h
  // ==========================================================================

  malloc: Runtime.staticAlloc,
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

  abs: 'Math.abs', // XXX should be integer?

  atoi: function(s) {
    return Math.floor(Number(Pointer_stringify(s)));
  },

  exit__deps: ['_exit'],
  exit: function(status) {
    __exit(status);
  },

  atexit: function(func, arg) {
    __ATEXIT__.push({ func: func, arg: arg });
  },
  __cxa_atexit: 'atexit',

  abort: function(code) {
    ABORT = true;
    throw 'ABORT: ' + code + ', at ' + (new Error().stack);
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

  getenv: function(name_) {
    return 0; // TODO
  },

  strtod__deps: ['isspace', 'isdigit'],
  strtod: function(str, endptr) {
    // Skip space.
    while (_isspace(str)) str++;

    var chr;
    var ret = 0;

    // Get whole part.
    while(1) {
      chr = {{{ makeGetValue('str', 0, 'i8') }}};
      if (!_isdigit(chr)) break;
      ret = ret*10 + chr - '0'.charCodeAt(0);
      str++;
    }

    // Get fractional part.
    if ({{{ makeGetValue('str', 0, 'i8') }}} == '.'.charCodeAt(0)) {
      str++;
      var mul = 1/10;
      while(1) {
        chr = {{{ makeGetValue('str', 0, 'i8') }}};
        if (!_isdigit(chr)) break;
        ret += mul*(chr - '0'.charCodeAt(0));
        mul /= 10;
        str++;
      }
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

    return ret;
  },

  qsort__deps: ['memcpy'],
  qsort: function(base, num, size, comparator) {
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

  // ==========================================================================
  // string.h
  // ==========================================================================

  memcpy: function (dest, src, num, idunno) {
#if ASSERTIONS
    assert(num % 1 === 0, 'memcpy given ' + num + ' bytes to copy. Problem with QUANTUM_SIZE=1 corrections perhaps?');
#endif
    // || 0, since memcpy sometimes copies uninitialized areas XXX: Investigate why initializing alloc'ed memory does not fix that too
    {{{ makeCopyValues('dest', 'src', 'num', 'null', ' || 0') }}};
  },
  llvm_memcpy_i32: 'memcpy',
  llvm_memcpy_i64: 'memcpy',
  llvm_memcpy_p0i8_p0i8_i32: 'memcpy',
  llvm_memcpy_p0i8_p0i8_i64: 'memcpy',

  memmove__deps: ['memcpy'],
  memmove: function(dest, src, num, idunno) {
    // not optimized!
    if (num === 0) return; // will confuse malloc if 0
    var tmp = _malloc(num);
    _memcpy(tmp, src, num);
    _memcpy(dest, tmp, num);
    _free(tmp);
  },
  llvm_memmove_i32: 'memmove',
  llvm_memmove_i64: 'memmove',
  llvm_memmove_p0i8_p0i8_i32: 'memmove',
  llvm_memmove_p0i8_p0i8_i64: 'memmove',

  memset: function(ptr, value, num) {
    {{{ makeSetValues('ptr', '0', 'value', 'null', 'num') }}}
  },
  llvm_memset_i32: 'memset',
  llvm_memset_p0i8_i32: 'memset',
  llvm_memset_p0i8_i64: 'memset',

  strlen: function(ptr) {
    return String_len(ptr);
  },

  strspn: function(pstr, pset) {
    var str = String_copy(pstr, true);
    var set = String_copy(pset);
    var i = 0;
    while (set.indexOf(str[i]) != -1) i++; // Must halt, as 0 is in str but not set
    return i;
  },

  strcspn: function(pstr, pset) {
    var str = String_copy(pstr, true);
    var set = String_copy(pset, true);
    var i = 0;
    while (set.indexOf(str[i]) == -1) i++; // Must halt, as 0 is in both
    return i;
  },

  strcpy: function(pdest, psrc) {
    var i = 0;
    do {
      {{{ makeCopyValues('pdest+i', 'psrc+i', 1, 'i8') }}}
      i ++;
    } while ({{{ makeGetValue('psrc', 'i-1', 'i8') }}} != 0);
    return pdest;
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

  strcat__deps: ['strlen'],
  strcat: function(pdest, psrc) {
    var len = _strlen(pdest);
    var i = 0;
    do {
      {{{ makeCopyValues('pdest+len+i', 'psrc+i', 1, 'i8') }}}
      i ++;
    } while ({{{ makeGetValue('psrc', 'i-1', 'i8') }}} != 0);
    return pdest;
  },

  strncat__deps: ['strlen'],
  strncat: function(pdest, psrc, num) {
    var len = _strlen(pdest);
    var i = 0;
    while(1) {
      {{{ makeCopyValues('pdest+len+i', 'psrc+i', 1, 'i8') }}}
      if ({{{ makeGetValue('pdest', 'len+i', 'i8') }}} == 0) break;
      i ++;
      if (i == num) {
        {{{ makeSetValue('pdest', 'len+i', 0, 'i8') }}}
        break;
      }
    }
    return pdest;
  },

  strtol: function(ptr) {
    assert(!arguments[1] && !arguments[2], "We don't support all strtol params yet");
    return parseInt(Pointer_stringify(ptr));
  },

  strcmp__deps: ['strncmp'],
  strcmp: function(px, py) {
    return _strncmp(px, py, TOTAL_MEMORY);
  },

  strcasecmp__deps: ['strncasecmp'],
  strcasecmp: function(px, py) {
    return _strncasecmp(px, py, TOTAL_MEMORY);
  },

  strncmp: function(px, py, n) {
    var i = 0;
    while (i < n) {
      var x = {{{ makeGetValue('px', 'i', 'i8') }}};
      var y = {{{ makeGetValue('py', 'i', 'i8') }}};
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
      var x = _tolower({{{ makeGetValue('px', 'i', 'i8') }}});
      var y = _tolower({{{ makeGetValue('py', 'i', 'i8') }}});
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
      var v1 = {{{ makeGetValue('p1', 'i', 'i8') }}};
      var v2 = {{{ makeGetValue('p2', 'i', 'i8') }}};
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
    var str1 = Pointer_stringify(ptr1);
    var str2 = Pointer_stringify(ptr2);
    var ret = str1.search(str2);
    return ret >= 0 ? ptr1 + ret : 0;
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
    return allocate(String_copy(ptr, true), 'i8', ALLOC_NORMAL);
  },

  strpbrk: function(ptr1, ptr2) {
    var searchSet = Runtime.set.apply(null, String_copy(ptr2));
    while ({{{ makeGetValue('ptr1', 0, 'i8') }}}) {
      if ({{{ makeGetValue('ptr1', 0, 'i8') }}} in searchSet) return ptr1;
      ptr1++;
    }
    return 0;
  },

  // Compiled from newlib; for the original source and licensing, see library_strtok_r.c XXX will not work with typed arrays
  strtok_r: function(b,j,f){var a;a=null;var c,e;b=b;var i=b!=0;a:do if(i)a=0;else{b=HEAP[f];if(b!=0){a=0;break a}c=0;a=3;break a}while(0);if(a==0){a:for(;;){e=HEAP[b];b+=1;a=j;var g=e;i=a;a=2;b:for(;;){d=a==5?d:0;a=HEAP[i+d];if(a!=0==0){a=9;break a}var d=d+1;if(g==a)break b;else a=5}a=2}if(a==9)if(g==0)c=HEAP[f]=0;else{c=b+-1;a:for(;;){e=HEAP[b];b+=1;a=j;g=e;d=a;a=10;b:for(;;){h=a==13?h:0;a=HEAP[d+h];if(a==g!=0)break a;var h=h+1;if(a!=0)a=13;else break b}}if(e==0)b=0;else HEAP[b+-1]=0; HEAP[f]=b;c=c}else if(a==7){HEAP[f]=b;HEAP[b+-1]=0;c=b+-1}}return c},

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

  isdigit: function(chr) {
    return chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0);
  },

  isxdigit: function(chr) {
    return (chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0)) ||
           (chr >= 'a'.charCodeAt(0) && chr <= 'f'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'F'.charCodeAt(0));
  },

  isalpha: function(chr) {
    return (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0));
  },

  isalnum: function(chr) {
    return (chr >= '0'.charCodeAt(0) && chr <= '9'.charCodeAt(0)) ||
           (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) ||
           (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0));
  },

  isspace: function(chr) {
    return chr in { 32: 0, 9: 0, 10: 0, 11: 0, 12: 0, 13: 0 };
  },

  iscntrl: function(chr) {
    return (chr >= 0 && chr <= 0x1f) || chr === 0x7f;
  },

  isprint__deps: ['iscntrl'],
  isprint: function(chr) {
    return !_iscntrl(chr);
  },

  toupper: function(chr) {
    if (chr >= 'a'.charCodeAt(0) && chr <= 'z'.charCodeAt(0)) {
      return chr - 'a'.charCodeAt(0) + 'A'.charCodeAt(0);
    }
    return chr;
  },

  tolower: function(chr) {
    if (chr >= 'A'.charCodeAt(0) && chr <= 'Z'.charCodeAt(0)) {
      return chr - 'A'.charCodeAt(0) + 'a'.charCodeAt(0);
    }
    return chr;
  },

  // ==========================================================================
  // ctype.h Linux specifics
  // ==========================================================================

  __ctype_b_loc: function() { // http://refspecs.freestandards.org/LSB_3.0.0/LSB-Core-generic/LSB-Core-generic/baselib---ctype-b-loc.html
    var me = ___ctype_b_loc;
    if (!me.ret) {
      var values = [
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,8195,0,8194,0,8194,0,8194,0,8194,0,2,0,2,
        0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,24577,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,
        0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,55304,0,55304,0,55304,0,55304,0,55304,0,55304,
        0,55304,0,55304,0,55304,0,55304,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,54536,0,54536,0,54536,0,54536,
        0,54536,0,54536,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,
        0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,50440,0,49156,0,49156,0,49156,0,49156,0,49156,0,49156,0,54792,0,54792,
        0,54792,0,54792,0,54792,0,54792,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,
        0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,50696,0,49156,0,49156,0,49156,0,49156,0,2,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0
      ];
      me.ret = allocate([allocate(values, 'i16', ALLOC_STATIC)+256], 'void*', ALLOC_STATIC);
#if USE_TYPED_ARRAYS == 0
      assert(HEAP[HEAP[me.ret]] == 2);
      assert(HEAP[HEAP[me.ret]-2] == 0);
      assert(HEAP[HEAP[me.ret]+18] == 8195);
#endif
    }
    return me.ret;
  },

  // LLVM specifics

  llvm_va_copy: function(ppdest, ppsrc) {
    {{{ makeCopyValues('ppdest', 'ppsrc', QUANTUM_SIZE, 'null') }}}
    /* Alternate implementation that copies the actual DATA; it assumes the va_list is prefixed by its size
    var psrc = IHEAP[ppsrc]-1;
    var num = IHEAP[psrc]; // right before the data, is the number of (flattened) values
    var pdest = _malloc(num+1);
    _memcpy(pdest, psrc, num+1);
    IHEAP[ppdest] = pdest+1;
    */
  },

  llvm_bswap_i32: function(x) {
    x = unSign(x, 32);
    var bytes = [];
    for (var i = 0; i < 4; i++) {
      bytes[i] = x & 255;
      x >>= 8;
    }
    var ret = 0;
    for (i = 0; i < 4; i++) {
      ret <<= 8;
      ret += bytes[i];
    }
    return ret;
  },

  __assert_fail: function(condition, file, line) {
    ABORT = true;
    throw 'Assertion failed: ' + Pointer_stringify(condition);//JSON.stringify(arguments)//condition;
  },

  __cxa_guard_acquire: function() {
    return 1;
  },
  __cxa_guard_release: function() {
    return 1;
  },

  // Exceptions - minimal support, only (...) for now (no actual exception objects can be caught)
  __cxa_allocate_exception: function(size) {
    return _malloc(size); // warning: leaked
  },
  __cxa_throw: function(ptr, data, dunno) {
#if EXCEPTION_DEBUG
    print('Compiled code throwing an exception, ' + [ptr,data,dunno] + ', at ' + new Error().stack);
#endif
    throw ptr;
  },
  llvm_eh_exception: function() {
    return 'code-generated exception: ' + (new Error().stack);
  },
  llvm_eh_selector: function(exception, personality, num) {
    return 0;
  },
  __cxa_begin_catch: function(ptr) {
  },
  __cxa_end_catch: function(ptr) {
  },

  __cxa_call_unexpected: function(exception) {
    ABORT = true;
    throw exception;
  },

  __gxx_personality_v0: function() {
  },

  llvm_umul_with_overflow_i32: function(x, y) {
    return {
      f0: x*y,
      f1: 0 // We never overflow... for now
    };
  },

  llvm_stacksave: function() {
    var self = _llvm_stacksave;
    if (!self.LLVM_SAVEDSTACKS) {
      self.LLVM_SAVEDSTACKS = [];
    }
    self.LLVM_SAVEDSTACKS.push(STACKTOP);
    return self.LLVM_SAVEDSTACKS.length-1;
  },
  llvm_stackrestore: function(p) {
    var self = _llvm_stacksave;
    var ret = self.LLVM_SAVEDSTACKS[p];
    self.LLVM_SAVEDSTACKS.splice(p, 1);
    return ret;
  },

  __cxa_pure_virtual: function() {
    ABORT = true;
    throw 'Pure virtual function called!';
  },

  llvm_flt_rounds: function() {
    return -1; // 'indeterminable' for FLT_ROUNDS
  },

  // iostream

  _ZNSt8ios_base4InitC1Ev: function() {
    // need valid 'file descriptors'
    //__ZSt4cout = 1;
    //__ZSt4cerr = 2;
  },
  _ZNSt8ios_base4InitD1Ev: '_ZNSt8ios_base4InitC1Ev',
  _ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_: 0, // endl
  _ZNSolsEi: function(stream, data) {
    __print__(data);
  },
  _ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc: function(stream, data) {
    __print__(Pointer_stringify(data));
  },
  _ZNSolsEd: function(stream, data) {
    __print__('\n');
  },
  _ZNSolsEPFRSoS_E: function(stream, data) {
    __print__('\n');
  },
  _ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_i: function(stream, data, call_) {
    __print__(Pointer_stringify(data));
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
  sqrt: 'Math.sqrt',
  sqrtf: 'Math.sqrt',
  fabs: 'Math.abs',
  fabsf: 'Math.abs',
  ceil: 'Math.ceil',
  ceilf: 'Math.ceil',
  floor: 'Math.floor',
  floorf: 'Math.floor',
  llvm_sqrt_f32: 'Math.sqrt',
  llvm_sqrt_f64: 'Math.sqrt',
  llvm_pow_f32: 'Math.pow',
  llvm_pow_f64: 'Math.pow',
  llvm_log_f32: 'Math.log',
  llvm_log_f64: 'Math.log',
  ldexp: function(x, exp_) {
    return x*Math.pow(2, exp_);
  },

  modf: function(x, intpart) {
    {{{ makeSetValue('intpart', 0, 'Math.floor(x)', 'double') }}}
    return x - {{{ makeGetValue('intpart', 0, 'double') }}};
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
      if (a<0 === b<0) return a;
      return -a;
  },

  hypot: function(a, b) {
     return Math.sqrt(a*a + b*b);
  },

  sinh: function(x) {
    var p = Math.pow(Math.E, x);
    return (p - (1 / p)) / 2;
  },

  cosh: function(x) {
    var p = Math.pow(Math.E, x);
    return (p + (1 / p)) / 2;
  },

  tanh__deps: ['sinh', 'cosh'],
  tanh: function(x) {
    return _sinh(x) / _cosh(x);
  },

  asinh: function(x) {
    return Math.log(x + Math.sqrt(x * x + 1));
  },

  acosh: function(x) {
    return Math.log(x * 1 + Math.sqrt(x * x - 1));
  },

  atanh: function(x) {
    return Math.log((1 + x) / (1 - x)) / 2;
  },

  // LLVM internal math

  exp2: function(x) {
    return Math.pow(2, x);
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
    isError: false,
    loadedLibs: {}, // handle -> [refcount, name, lib_object]
    loadedLibNames: {}, // name -> handle
  },
  // void* dlopen(const char* filename, int flag);
  dlopen__deps: ['$DLFCN_DATA'],
  dlopen: function(filename, flag) {
    // TODO: Add support for LD_LIBRARY_PATH.
    filename = Pointer_stringify(filename);

    if (DLFCN_DATA.loadedLibNames[filename]) {
      // Already loaded; increment ref count and return.
      var handle = DLFCN_DATA.loadedLibNames[filename];
      DLFCN_DATA.loadedLibs[handle][0]++;
      return handle;
    }

    try {
      var lib_data = read(filename);
    } catch (e) {
      DLFCN_DATA.isError = true;
      return 0;
    }

    try {
      var lib_module = eval(lib_data)(FUNCTION_TABLE.length);
    } catch (e) {
#if ASSERTIONS
      print('Error in loading dynamic library: ' + e);
#endif
      DLFCN_DATA.isError = true;
      return 0;
    }

    // Not all browsers support Object.keys().
    var handle = 1;
    for (var key in DLFCN_DATA.loadedLibs) {
      if (DLFCN_DATA.loadedLibs.hasOwnProperty(key)) handle++;
    }

    DLFCN_DATA.loadedLibs[handle] = [1, filename, lib_module];
    DLFCN_DATA.loadedLibNames[filename] = handle;

    // We don't care about RTLD_NOW and RTLD_LAZY.
    if (flag & 256) { // RTLD_GLOBAL
      for (var ident in lib_module) {
        if (lib_module.hasOwnProperty(ident)) {
          // TODO: Check if we need to unmangle here.
          Module[ident] = lib_module[ident];
        }
      }
    }

    return handle;
  },
  // int dlclose(void* handle);
  dlclose__deps: ['$DLFCN_DATA'],
  dlclose: function(handle) {
    if (!DLFCN_DATA.loadedLibs[handle]) {
      DLFCN_DATA.isError = true;
      return 1;
    } else {
      var lib_record = DLFCN_DATA.loadedLibs[handle];
      if (lib_record[0]-- == 0) {
        delete DLFCN_DATA.loadedLibNames[lib_record[1]];
        delete DLFCN_DATA.loadedLibs[handle];
      }
      return 0;
    }
  },
  // void* dlsym(void* handle, const char* symbol);
  dlsym__deps: ['$DLFCN_DATA'],
  dlsym: function(handle, symbol) {
    symbol = Pointer_stringify(symbol);
    // TODO: Properly mangle.
    symbol = '_' + symbol;

    if (!DLFCN_DATA.loadedLibs[handle]) {
      DLFCN_DATA.isError = true;
      return 0;
    } else {
      var lib_module = DLFCN_DATA.loadedLibs[handle][2];
      if (!lib_module[symbol]) {
        DLFCN_DATA.isError = true;
        return 0;
      } else {
        var result = lib_module[symbol];
        if (typeof result == 'function') {
          // TODO: Cache functions rather than appending on every lookup.
          FUNCTION_TABLE.push(result);
          FUNCTION_TABLE.push(0);
          result = FUNCTION_TABLE.length - 2;
        }
        return result;
      }
    }
  },
  // char* dlerror(void);
  dlerror__deps: ['$DLFCN_DATA'],
  dlerror: function() {
    if (DLFCN_DATA.isError) {
      return 0;
    } else {
      // TODO: Return non-generic error messages.
      if (DLFCN_DATA.error === null) {
        var msg = 'An error occurred while loading dynamic library.';
        var arr = Module.intArrayFromString(msg)
        DLFCN_DATA.error = allocate(arr, 'i8', 2);
      }
      DLFCN_DATA.isError = false;
      return DLFCN_DATA.error;
    }
  },

  // ==========================================================================
  // System calls
  // ==========================================================================

  sbrk: function(bytes) {
    // Implement a Linux-like 'memory area' for our 'process'.
    // Changes the size of the memory area by |bytes|; returns the
    // address of the previous top ('break') of the memory area

    // We need to make sure no one else allocates unfreeable memory!
    // We must control this entirely. So we don't even need to do
    // unfreeable allocations - the HEAP is ours, from STATICTOP up.
    // TODO: We could in theory slice off the top of the HEAP when
    // sbrk gets a negative increment in |bytes|...
    var self = _sbrk;
    if (!self.STATICTOP) {
      STATICTOP = alignMemoryPage(STATICTOP);
      self.STATICTOP = STATICTOP;
      self.DATASIZE = 0;
    } else {
      assert(self.STATICTOP == STATICTOP, "Noone should touch the heap!");
    }
    var ret = STATICTOP + self.DATASIZE;
    self.DATASIZE += alignMemoryPage(bytes);
    return ret; // previous break location
  },

  // ==========================================================================
  // pwd.h
  // ==========================================================================

  getpwuid: function(uid) {
    return 0; // NULL
  },

  // ==========================================================================
  // time.h
  // ==========================================================================

  clock: function() {
    if (_clock.start === undefined) _clock.start = new Date();
    return (Date.now() - _clock.start.getTime()) * 1000;
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

  __tm_struct_layout: Runtime.generateStructInfo(null, '%struct.tm'),
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
    var offset = {{{ makeGetValue('_timezone', 0, 'i32') }}};
    var daylight = {{{ makeGetValue('_daylight', 0, 'i32') }}};
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

    var timezone = date.toString().match(/\(([A-Z]+)\)/)[1];
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
  tzname: null,
  daylight: null,
  timezone: null,
  tzset__deps: ['malloc', 'tzname', 'daylight', 'timezone'],
  tzset: function() {
    // TODO: Use (malleable) environment variables instead of system settings.
    if (_tzname !== null) return;

    _timezone = _malloc(QUANTUM_SIZE);
    {{{ makeSetValue('_timezone', '0', '-(new Date()).getTimezoneOffset() * 60', 'i32') }}}

    _daylight = _malloc(QUANTUM_SIZE);
    var winter = new Date(2000, 0, 1);
    var summer = new Date(2000, 6, 1);
    {{{ makeSetValue('_daylight', '0', 'Number(winter.getTimezoneOffset() != summer.getTimezoneOffset())', 'i32') }}}

    var winterName = winter.toString().match(/\(([A-Z]+)\)/)[1];
    var summerName = summer.toString().match(/\(([A-Z]+)\)/)[1];
    var winterNamePtr = allocate(intArrayFromString(winterName), 'i8', ALLOC_NORMAL);
    var summerNamePtr = allocate(intArrayFromString(summerName), 'i8', ALLOC_NORMAL);
    _tzname = _malloc(2 * QUANTUM_SIZE);
    {{{ makeSetValue('_tzname', '0', 'winterNamePtr', 'i32') }}}
    {{{ makeSetValue('_tzname', QUANTUM_SIZE, 'summerNamePtr', 'i32') }}}
  },

  stime: function(when) {
    // TODO: Set errno.
    return -1;
  },

  // TODO: Implement strftime(), strptime() and getdate().

  // ==========================================================================
  // sys/time.h
  // ==========================================================================

  gettimeofday: function(ptr) {
    // %struct.timeval = type { i32, i32 }
    var indexes = Runtime.calculateStructAlignment({ fields: ['i32', 'i32'] });
    var now = Date.now();
    {{{ makeSetValue('ptr', 'indexes[0]', 'Math.floor(now/1000)', 'i32') }}} // seconds
    {{{ makeSetValue('ptr', 'indexes[1]', 'Math.floor((now-1000*Math.floor(now/1000))*1000)', 'i32') }}} // microseconds
    return 0;
  },

  // ==========================================================================
  // setjmp.h
  // ==========================================================================

  _setjmp: function(env) {
    // XXX print('WARNING: setjmp() not really implemented, will fail if longjmp() is actually called');
    return 0;
  },

  _longjmp: function(env, val) {
    // not really working...
    assert(0);
  },

  // ==========================================================================
  // signal.h
  // ==========================================================================

  signal: function(sig, func) {
    // TODO
    return 0;
  },

  __libc_current_sigrtmin: function() {
    return 0;
  },
  __libc_current_sigrtmax: function() {
    return 0;
  },

  // ==========================================================================
  // locale.h
  // ==========================================================================

  setlocale: function(category, locale) {
    return 0;
  },

  localeconv: function() {
    // %struct.timeval = type { char* decimal point, other stuff... }
    // var indexes = Runtime.calculateStructAlignment({ fields: ['i32', 'i32'] });
    var me = _localeconv;
    if (!me.ret) {
      me.ret = allocate([allocate(intArrayFromString('.'), 'i8', ALLOC_NORMAL)], 'i8', ALLOC_NORMAL); // just decimal point, for now
    }
    return me.ret;
  },

  // ==========================================================================
  // langinfo.h
  // ==========================================================================

  nl_langinfo: function(item) {
    var me = _nl_langinfo;
    if (!me.ret) {
      me.ret = allocate(intArrayFromString("eh?"), 'i8', ALLOC_NORMAL);
    }
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
  __setErrNo: function(value) {
    // For convenient setting and returning of errno.
    var me = ___setErrNo;
    if (!me.ptr) me.ptr = allocate([0], 'i32', ALLOC_STATIC);
    {{{ makeSetValue('me.ptr', '0', 'value', 'i32') }}}
    return value;
  },
  __errno_location__deps: ['__setErrNo'],
  __errno_location: function() {
    if (!___setErrNo.ptr) ___setErrNo(0);
    return ___setErrNo.ptr;
  },

  // ==========================================================================
  // pthread.h (stubs for mutexes only - no thread support yet!)
  // ==========================================================================

  pthread_mutex_init: function() {},
  pthread_mutex_destroy: function() {},
  pthread_mutex_lock: function() {},
  pthread_mutex_unlock: function() {},

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
  // ** emscripten.h **
  // ==========================================================================
  emscripten_run_script: function(ptr) {
    eval(Pointer_stringify(ptr));
  },

  _Z21emscripten_run_scriptPKc: function(ptr) {
    eval(Pointer_stringify(ptr));
  }
};

