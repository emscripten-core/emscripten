mergeInto(LibraryManager.library, {
  $FS__deps: ['$ERRNO_CODES', '$ERRNO_MESSAGES', '__setErrNo', '$PATH', '$TTY', '$MEMFS',
#if __EMSCRIPTEN_HAS_idbfs_js__
    '$IDBFS',
#endif
#if __EMSCRIPTEN_HAS_nodefs_js__
    '$NODEFS',
#endif
#if __EMSCRIPTEN_HAS_workerfs_js__
    '$WORKERFS',
#endif
    'stdin', 'stdout', 'stderr'],
  $FS__postset: 'FS.staticInit();' +
                '__ATINIT__.unshift(function() { if (!Module["noFSInit"] && !FS.init.initialized) FS.init() });' +
                '__ATMAIN__.push(function() { FS.ignorePermissions = false });' +
                '__ATEXIT__.push(function() { FS.quit() });' +
                //Get module methods from settings
                '{{{ EXPORTED_RUNTIME_METHODS.filter(function(func) { return func.substr(0, 3) === 'FS_' }).map(function(func){return 'Module["' + func + '"] = FS.' + func.substr(3) + ";"}).reduce(function(str, func){return str + func;}, '') }}}',
  $FS: {
    root: null,
    mounts: [],
    devices: [null],
    streams: [],
    nextInode: 1,
    nameTable: null,
    currentPath: '/',
    initialized: false,
    // Whether we are currently ignoring permissions. Useful when preparing the
    // filesystem and creating files inside read-only folders.
    // This is set to false when the runtime is initialized, allowing you
    // to modify the filesystem freely before run() is called.
    ignorePermissions: true,
    trackingDelegate: {},
    tracking: {
      openFlags: {
        READ: 1 << 0,
        WRITE: 1 << 1
      }
    },
    ErrnoError: null, // set during init
    genericErrors: {},
    filesystems: null,
    syncFSRequests: 0, // we warn if there are multiple in flight at once

    handleFSError: function(e) {
      if (!(e instanceof FS.ErrnoError)) throw e + ' : ' + stackTrace();
      return ___setErrNo(e.errno);
    },

    //
    // paths
    //
    lookupPath: function(path, opts) {
      path = PATH.resolve(FS.cwd(), path);
      opts = opts || {};

      if (!path) return { path: '', node: null };

      var defaults = {
        follow_mount: true,
        recurse_count: 0
      };
      for (var key in defaults) {
        if (opts[key] === undefined) {
          opts[key] = defaults[key];
        }
      }

      if (opts.recurse_count > 8) {  // max recursive lookup of 8
        throw new FS.ErrnoError(ERRNO_CODES.ELOOP);
      }

      // split the path
      var parts = PATH.normalizeArray(path.split('/').filter(function(p) {
        return !!p;
      }), false);

      // start at the root
      var current = FS.root;
      var current_path = '/';

      for (var i = 0; i < parts.length; i++) {
        var islast = (i === parts.length-1);
        if (islast && opts.parent) {
          // stop resolving
          break;
        }

        current = FS.lookupNode(current, parts[i]);
        current_path = PATH.join2(current_path, parts[i]);

        // jump to the mount's root node if this is a mountpoint
        if (FS.isMountpoint(current)) {
          if (!islast || (islast && opts.follow_mount)) {
            current = current.mounted.root;
          }
        }

        // by default, lookupPath will not follow a symlink if it is the final path component.
        // setting opts.follow = true will override this behavior.
        if (!islast || opts.follow) {
          var count = 0;
          while (FS.isLink(current.mode)) {
            var link = FS.readlink(current_path);
            current_path = PATH.resolve(PATH.dirname(current_path), link);

            var lookup = FS.lookupPath(current_path, { recurse_count: opts.recurse_count });
            current = lookup.node;

            if (count++ > 40) {  // limit max consecutive symlinks to 40 (SYMLOOP_MAX).
              throw new FS.ErrnoError(ERRNO_CODES.ELOOP);
            }
          }
        }
      }

      return { path: current_path, node: current };
    },
    getPath: function(node) {
      var path;
      while (true) {
        if (FS.isRoot(node)) {
          var mount = node.mount.mountpoint;
          if (!path) return mount;
          return mount[mount.length-1] !== '/' ? mount + '/' + path : mount + path;
        }
        path = path ? node.name + '/' + path : node.name;
        node = node.parent;
      }
    },

    //
    // nodes
    //
    hashName: function(parentid, name) {
      var hash = 0;

#if CASE_INSENSITIVE_FS
      name = name.toLowerCase();
#endif

      for (var i = 0; i < name.length; i++) {
        hash = ((hash << 5) - hash + name.charCodeAt(i)) | 0;
      }
      return ((parentid + hash) >>> 0) % FS.nameTable.length;
    },
    hashAddNode: function(node) {
      var hash = FS.hashName(node.parent.id, node.name);
      node.name_next = FS.nameTable[hash];
      FS.nameTable[hash] = node;
    },
    hashRemoveNode: function(node) {
      var hash = FS.hashName(node.parent.id, node.name);
      if (FS.nameTable[hash] === node) {
        FS.nameTable[hash] = node.name_next;
      } else {
        var current = FS.nameTable[hash];
        while (current) {
          if (current.name_next === node) {
            current.name_next = node.name_next;
            break;
          }
          current = current.name_next;
        }
      }
    },
    lookupNode: function(parent, name) {
      var err = FS.mayLookup(parent);
      if (err) {
        throw new FS.ErrnoError(err, parent);
      }
      var hash = FS.hashName(parent.id, name);
#if CASE_INSENSITIVE_FS
      name = name.toLowerCase();
#endif
      for (var node = FS.nameTable[hash]; node; node = node.name_next) {
        var nodeName = node.name;
#if CASE_INSENSITIVE_FS
        nodeName = nodeName.toLowerCase();
#endif
        if (node.parent.id === parent.id && nodeName === name) {
          return node;
        }
      }
      // if we failed to find it in the cache, call into the VFS
      return FS.lookup(parent, name);
    },
    createNode: function(parent, name, mode, rdev) {
      if (!FS.FSNode) {
        FS.FSNode = function(parent, name, mode, rdev) {
          if (!parent) {
            parent = this;  // root node sets parent to itself
          }
          this.parent = parent;
          this.mount = parent.mount;
          this.mounted = null;
          this.id = FS.nextInode++;
          this.name = name;
          this.mode = mode;
          this.node_ops = {};
          this.stream_ops = {};
          this.rdev = rdev;
        };

        FS.FSNode.prototype = {};

        // compatibility
        var readMode = {{{ cDefine('S_IRUGO') }}} | {{{ cDefine('S_IXUGO') }}};
        var writeMode = {{{ cDefine('S_IWUGO') }}};

        // NOTE we must use Object.defineProperties instead of individual calls to
        // Object.defineProperty in order to make closure compiler happy
        Object.defineProperties(FS.FSNode.prototype, {
          read: {
            get: function() { return (this.mode & readMode) === readMode; },
            set: function(val) { val ? this.mode |= readMode : this.mode &= ~readMode; }
          },
          write: {
            get: function() { return (this.mode & writeMode) === writeMode; },
            set: function(val) { val ? this.mode |= writeMode : this.mode &= ~writeMode; }
          },
          isFolder: {
            get: function() { return FS.isDir(this.mode); }
          },
          isDevice: {
            get: function() { return FS.isChrdev(this.mode); }
          }
        });
      }

      var node = new FS.FSNode(parent, name, mode, rdev);

      FS.hashAddNode(node);

      return node;
    },
    destroyNode: function(node) {
      FS.hashRemoveNode(node);
    },
    isRoot: function(node) {
      return node === node.parent;
    },
    isMountpoint: function(node) {
      return !!node.mounted;
    },
    isFile: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFREG') }}};
    },
    isDir: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFDIR') }}};
    },
    isLink: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFLNK') }}};
    },
    isChrdev: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFCHR') }}};
    },
    isBlkdev: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFBLK') }}};
    },
    isFIFO: function(mode) {
      return (mode & {{{ cDefine('S_IFMT') }}}) === {{{ cDefine('S_IFIFO') }}};
    },
    isSocket: function(mode) {
      return (mode & {{{ cDefine('S_IFSOCK') }}}) === {{{ cDefine('S_IFSOCK') }}};
    },

    //
    // permissions
    //
    flagModes: {
      '"r"': {{{ cDefine('O_RDONLY') }}},
      '"rs"': {{{ cDefine('O_RDONLY') }}} | {{{ cDefine('O_SYNC') }}},
      '"r+"': {{{ cDefine('O_RDWR') }}},
      '"w"': {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_WRONLY') }}},
      '"wx"': {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_WRONLY') }}} | {{{ cDefine('O_EXCL') }}},
      '"xw"': {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_WRONLY') }}} | {{{ cDefine('O_EXCL') }}},
      '"w+"': {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_RDWR') }}},
      '"wx+"': {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_RDWR') }}} | {{{ cDefine('O_EXCL') }}},
      '"xw+"': {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_RDWR') }}} | {{{ cDefine('O_EXCL') }}},
      '"a"': {{{ cDefine('O_APPEND') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_WRONLY') }}},
      '"ax"': {{{ cDefine('O_APPEND') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_WRONLY') }}} | {{{ cDefine('O_EXCL') }}},
      '"xa"': {{{ cDefine('O_APPEND') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_WRONLY') }}} | {{{ cDefine('O_EXCL') }}},
      '"a+"': {{{ cDefine('O_APPEND') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_RDWR') }}},
      '"ax+"': {{{ cDefine('O_APPEND') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_RDWR') }}} | {{{ cDefine('O_EXCL') }}},
      '"xa+"': {{{ cDefine('O_APPEND') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_RDWR') }}} | {{{ cDefine('O_EXCL') }}}
    },
    // convert the 'r', 'r+', etc. to it's corresponding set of O_* flags
    modeStringToFlags: function(str) {
      var flags = FS.flagModes[str];
      if (typeof flags === 'undefined') {
        throw new Error('Unknown file open mode: ' + str);
      }
      return flags;
    },
    // convert O_* bitmask to a string for nodePermissions
    flagsToPermissionString: function(flag) {
      var perms = ['r', 'w', 'rw'][flag & 3];
      if ((flag & {{{ cDefine('O_TRUNC') }}})) {
        perms += 'w';
      }
      return perms;
    },
    nodePermissions: function(node, perms) {
      if (FS.ignorePermissions) {
        return 0;
      }
      // return 0 if any user, group or owner bits are set.
      if (perms.indexOf('r') !== -1 && !(node.mode & {{{ cDefine('S_IRUGO') }}})) {
        return ERRNO_CODES.EACCES;
      } else if (perms.indexOf('w') !== -1 && !(node.mode & {{{ cDefine('S_IWUGO') }}})) {
        return ERRNO_CODES.EACCES;
      } else if (perms.indexOf('x') !== -1 && !(node.mode & {{{ cDefine('S_IXUGO') }}})) {
        return ERRNO_CODES.EACCES;
      }
      return 0;
    },
    mayLookup: function(dir) {
      var err = FS.nodePermissions(dir, 'x');
      if (err) return err;
      if (!dir.node_ops.lookup) return ERRNO_CODES.EACCES;
      return 0;
    },
    mayCreate: function(dir, name) {
      try {
        var node = FS.lookupNode(dir, name);
        return ERRNO_CODES.EEXIST;
      } catch (e) {
      }
      return FS.nodePermissions(dir, 'wx');
    },
    mayDelete: function(dir, name, isdir) {
      var node;
      try {
        node = FS.lookupNode(dir, name);
      } catch (e) {
        return e.errno;
      }
      var err = FS.nodePermissions(dir, 'wx');
      if (err) {
        return err;
      }
      if (isdir) {
        if (!FS.isDir(node.mode)) {
          return ERRNO_CODES.ENOTDIR;
        }
        if (FS.isRoot(node) || FS.getPath(node) === FS.cwd()) {
          return ERRNO_CODES.EBUSY;
        }
      } else {
        if (FS.isDir(node.mode)) {
          return ERRNO_CODES.EISDIR;
        }
      }
      return 0;
    },
    mayOpen: function(node, flags) {
      if (!node) {
        return ERRNO_CODES.ENOENT;
      }
      if (FS.isLink(node.mode)) {
        return ERRNO_CODES.ELOOP;
      } else if (FS.isDir(node.mode)) {
        if (FS.flagsToPermissionString(flags) !== 'r' || // opening for write
            (flags & {{{ cDefine('O_TRUNC') }}})) { // TODO: check for O_SEARCH? (== search for dir only)
          return ERRNO_CODES.EISDIR;
        }
      }
      return FS.nodePermissions(node, FS.flagsToPermissionString(flags));
    },

    //
    // streams
    //
    MAX_OPEN_FDS: 4096,
    nextfd: function(fd_start, fd_end) {
      fd_start = fd_start || 0;
      fd_end = fd_end || FS.MAX_OPEN_FDS;
      for (var fd = fd_start; fd <= fd_end; fd++) {
        if (!FS.streams[fd]) {
          return fd;
        }
      }
      throw new FS.ErrnoError(ERRNO_CODES.EMFILE);
    },
    getStream: function(fd) {
      return FS.streams[fd];
    },
    // TODO parameterize this function such that a stream
    // object isn't directly passed in. not possible until
    // SOCKFS is completed.
    createStream: function(stream, fd_start, fd_end) {
      if (!FS.FSStream) {
        FS.FSStream = function(){};
        FS.FSStream.prototype = {};
        // compatibility
        Object.defineProperties(FS.FSStream.prototype, {
          object: {
            get: function() { return this.node; },
            set: function(val) { this.node = val; }
          },
          isRead: {
            get: function() { return (this.flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_WRONLY') }}}; }
          },
          isWrite: {
            get: function() { return (this.flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_RDONLY') }}}; }
          },
          isAppend: {
            get: function() { return (this.flags & {{{ cDefine('O_APPEND') }}}); }
          }
        });
      }
      // clone it, so we can return an instance of FSStream
      var newStream = new FS.FSStream();
      for (var p in stream) {
        newStream[p] = stream[p];
      }
      stream = newStream;
      var fd = FS.nextfd(fd_start, fd_end);
      stream.fd = fd;
      FS.streams[fd] = stream;
      return stream;
    },
    closeStream: function(fd) {
      FS.streams[fd] = null;
    },

    //
    // devices
    //
    // each character device consists of a device id + stream operations.
    // when a character device node is created (e.g. /dev/stdin) it is
    // assigned a device id that lets us map back to the actual device.
    // by default, each character device stream (e.g. _stdin) uses chrdev_stream_ops.
    // however, once opened, the stream's operations are overridden with
    // the operations of the device its underlying node maps back to.
    chrdev_stream_ops: {
      open: function(stream) {
        var device = FS.getDevice(stream.node.rdev);
        // override node's stream ops with the device's
        stream.stream_ops = device.stream_ops;
        // forward the open call
        if (stream.stream_ops.open) {
          stream.stream_ops.open(stream);
        }
      },
      llseek: function() {
        throw new FS.ErrnoError(ERRNO_CODES.ESPIPE);
      }
    },
    major: function(dev) {
      return ((dev) >> 8);
    },
    minor: function(dev) {
      return ((dev) & 0xff);
    },
    makedev: function(ma, mi) {
      return ((ma) << 8 | (mi));
    },
    registerDevice: function(dev, ops) {
      FS.devices[dev] = { stream_ops: ops };
    },
    getDevice: function(dev) {
      return FS.devices[dev];
    },

    //
    // core
    //
    getMounts: function(mount) {
      var mounts = [];
      var check = [mount];

      while (check.length) {
        var m = check.pop();

        mounts.push(m);

        check.push.apply(check, m.mounts);
      }

      return mounts;
    },
    syncfs: function(populate, callback) {
      if (typeof(populate) === 'function') {
        callback = populate;
        populate = false;
      }

      FS.syncFSRequests++;

      if (FS.syncFSRequests > 1) {
        console.log('warning: ' + FS.syncFSRequests + ' FS.syncfs operations in flight at once, probably just doing extra work');
      }

      var mounts = FS.getMounts(FS.root.mount);
      var completed = 0;

      function doCallback(err) {
        assert(FS.syncFSRequests > 0);
        FS.syncFSRequests--;
        return callback(err);
      }

      function done(err) {
        if (err) {
          if (!done.errored) {
            done.errored = true;
            return doCallback(err);
          }
          return;
        }
        if (++completed >= mounts.length) {
          doCallback(null);
        }
      };

      // sync all mounts
      mounts.forEach(function (mount) {
        if (!mount.type.syncfs) {
          return done(null);
        }
        mount.type.syncfs(mount, populate, done);
      });
    },
    mount: function(type, opts, mountpoint) {
      var root = mountpoint === '/';
      var pseudo = !mountpoint;
      var node;

      if (root && FS.root) {
        throw new FS.ErrnoError(ERRNO_CODES.EBUSY);
      } else if (!root && !pseudo) {
        var lookup = FS.lookupPath(mountpoint, { follow_mount: false });

        mountpoint = lookup.path;  // use the absolute path
        node = lookup.node;

        if (FS.isMountpoint(node)) {
          throw new FS.ErrnoError(ERRNO_CODES.EBUSY);
        }

        if (!FS.isDir(node.mode)) {
          throw new FS.ErrnoError(ERRNO_CODES.ENOTDIR);
        }
      }

      var mount = {
        type: type,
        opts: opts,
        mountpoint: mountpoint,
        mounts: []
      };

      // create a root node for the fs
      var mountRoot = type.mount(mount);
      mountRoot.mount = mount;
      mount.root = mountRoot;

      if (root) {
        FS.root = mountRoot;
      } else if (node) {
        // set as a mountpoint
        node.mounted = mount;

        // add the new mount to the current mount's children
        if (node.mount) {
          node.mount.mounts.push(mount);
        }
      }

      return mountRoot;
    },
    unmount: function (mountpoint) {
      var lookup = FS.lookupPath(mountpoint, { follow_mount: false });

      if (!FS.isMountpoint(lookup.node)) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }

      // destroy the nodes for this mount, and all its child mounts
      var node = lookup.node;
      var mount = node.mounted;
      var mounts = FS.getMounts(mount);

      Object.keys(FS.nameTable).forEach(function (hash) {
        var current = FS.nameTable[hash];

        while (current) {
          var next = current.name_next;

          if (mounts.indexOf(current.mount) !== -1) {
            FS.destroyNode(current);
          }

          current = next;
        }
      });

      // no longer a mountpoint
      node.mounted = null;

      // remove this mount from the child mounts
      var idx = node.mount.mounts.indexOf(mount);
      assert(idx !== -1);
      node.mount.mounts.splice(idx, 1);
    },
    lookup: function(parent, name) {
      return parent.node_ops.lookup(parent, name);
    },
    // generic function for all node creation
    mknod: function(path, mode, dev) {
      var lookup = FS.lookupPath(path, { parent: true });
      var parent = lookup.node;
      var name = PATH.basename(path);
      if (!name || name === '.' || name === '..') {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      var err = FS.mayCreate(parent, name);
      if (err) {
        throw new FS.ErrnoError(err);
      }
      if (!parent.node_ops.mknod) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      return parent.node_ops.mknod(parent, name, mode, dev);
    },
    // helpers to create specific types of nodes
    create: function(path, mode) {
      mode = mode !== undefined ? mode : 438 /* 0666 */;
      mode &= {{{ cDefine('S_IALLUGO') }}};
      mode |= {{{ cDefine('S_IFREG') }}};
      return FS.mknod(path, mode, 0);
    },
    mkdir: function(path, mode) {
      mode = mode !== undefined ? mode : 511 /* 0777 */;
      mode &= {{{ cDefine('S_IRWXUGO') }}} | {{{ cDefine('S_ISVTX') }}};
      mode |= {{{ cDefine('S_IFDIR') }}};
      return FS.mknod(path, mode, 0);
    },
    // Creates a whole directory tree chain if it doesn't yet exist
    mkdirTree: function(path, mode) {
      var dirs = path.split('/');
      var d = '';
      for (var i = 0; i < dirs.length; ++i) {
        if (!dirs[i]) continue;
        d += '/' + dirs[i];
        try {
          FS.mkdir(d, mode);
        } catch(e) {
          if (e.errno != ERRNO_CODES.EEXIST) throw e;
        }
      }
    },
    mkdev: function(path, mode, dev) {
      if (typeof(dev) === 'undefined') {
        dev = mode;
        mode = 438 /* 0666 */;
      }
      mode |= {{{ cDefine('S_IFCHR') }}};
      return FS.mknod(path, mode, dev);
    },
    symlink: function(oldpath, newpath) {
      if (!PATH.resolve(oldpath)) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      }
      var lookup = FS.lookupPath(newpath, { parent: true });
      var parent = lookup.node;
      if (!parent) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      }
      var newname = PATH.basename(newpath);
      var err = FS.mayCreate(parent, newname);
      if (err) {
        throw new FS.ErrnoError(err);
      }
      if (!parent.node_ops.symlink) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      return parent.node_ops.symlink(parent, newname, oldpath);
    },
    rename: function(old_path, new_path) {
      var old_dirname = PATH.dirname(old_path);
      var new_dirname = PATH.dirname(new_path);
      var old_name = PATH.basename(old_path);
      var new_name = PATH.basename(new_path);
      // parents must exist
      var lookup, old_dir, new_dir;
      try {
        lookup = FS.lookupPath(old_path, { parent: true });
        old_dir = lookup.node;
        lookup = FS.lookupPath(new_path, { parent: true });
        new_dir = lookup.node;
      } catch (e) {
        throw new FS.ErrnoError(ERRNO_CODES.EBUSY);
      }
      if (!old_dir || !new_dir) throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      // need to be part of the same mount
      if (old_dir.mount !== new_dir.mount) {
        throw new FS.ErrnoError(ERRNO_CODES.EXDEV);
      }
      // source must exist
      var old_node = FS.lookupNode(old_dir, old_name);
      // old path should not be an ancestor of the new path
      var relative = PATH.relative(old_path, new_dirname);
      if (relative.charAt(0) !== '.') {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      // new path should not be an ancestor of the old path
      relative = PATH.relative(new_path, old_dirname);
      if (relative.charAt(0) !== '.') {
        throw new FS.ErrnoError(ERRNO_CODES.ENOTEMPTY);
      }
      // see if the new path already exists
      var new_node;
      try {
        new_node = FS.lookupNode(new_dir, new_name);
      } catch (e) {
        // not fatal
      }
      // early out if nothing needs to change
      if (old_node === new_node) {
        return;
      }
      // we'll need to delete the old entry
      var isdir = FS.isDir(old_node.mode);
      var err = FS.mayDelete(old_dir, old_name, isdir);
      if (err) {
        throw new FS.ErrnoError(err);
      }
      // need delete permissions if we'll be overwriting.
      // need create permissions if new doesn't already exist.
      err = new_node ?
        FS.mayDelete(new_dir, new_name, isdir) :
        FS.mayCreate(new_dir, new_name);
      if (err) {
        throw new FS.ErrnoError(err);
      }
      if (!old_dir.node_ops.rename) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      if (FS.isMountpoint(old_node) || (new_node && FS.isMountpoint(new_node))) {
        throw new FS.ErrnoError(ERRNO_CODES.EBUSY);
      }
      // if we are going to change the parent, check write permissions
      if (new_dir !== old_dir) {
        err = FS.nodePermissions(old_dir, 'w');
        if (err) {
          throw new FS.ErrnoError(err);
        }
      }
      try {
        if (FS.trackingDelegate['willMovePath']) {
          FS.trackingDelegate['willMovePath'](old_path, new_path);
        }
      } catch(e) {
        console.log("FS.trackingDelegate['willMovePath']('"+old_path+"', '"+new_path+"') threw an exception: " + e.message);
      }
      // remove the node from the lookup hash
      FS.hashRemoveNode(old_node);
      // do the underlying fs rename
      try {
        old_dir.node_ops.rename(old_node, new_dir, new_name);
      } catch (e) {
        throw e;
      } finally {
        // add the node back to the hash (in case node_ops.rename
        // changed its name)
        FS.hashAddNode(old_node);
      }
      try {
        if (FS.trackingDelegate['onMovePath']) FS.trackingDelegate['onMovePath'](old_path, new_path);
      } catch(e) {
        console.log("FS.trackingDelegate['onMovePath']('"+old_path+"', '"+new_path+"') threw an exception: " + e.message);
      }
    },
    rmdir: function(path) {
      var lookup = FS.lookupPath(path, { parent: true });
      var parent = lookup.node;
      var name = PATH.basename(path);
      var node = FS.lookupNode(parent, name);
      var err = FS.mayDelete(parent, name, true);
      if (err) {
        throw new FS.ErrnoError(err);
      }
      if (!parent.node_ops.rmdir) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      if (FS.isMountpoint(node)) {
        throw new FS.ErrnoError(ERRNO_CODES.EBUSY);
      }
      try {
        if (FS.trackingDelegate['willDeletePath']) {
          FS.trackingDelegate['willDeletePath'](path);
        }
      } catch(e) {
        console.log("FS.trackingDelegate['willDeletePath']('"+path+"') threw an exception: " + e.message);
      }
      parent.node_ops.rmdir(parent, name);
      FS.destroyNode(node);
      try {
        if (FS.trackingDelegate['onDeletePath']) FS.trackingDelegate['onDeletePath'](path);
      } catch(e) {
        console.log("FS.trackingDelegate['onDeletePath']('"+path+"') threw an exception: " + e.message);
      }
    },
    readdir: function(path) {
      var lookup = FS.lookupPath(path, { follow: true });
      var node = lookup.node;
      if (!node.node_ops.readdir) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOTDIR);
      }
      return node.node_ops.readdir(node);
    },
    unlink: function(path) {
      var lookup = FS.lookupPath(path, { parent: true });
      var parent = lookup.node;
      var name = PATH.basename(path);
      var node = FS.lookupNode(parent, name);
      var err = FS.mayDelete(parent, name, false);
      if (err) {
        // According to POSIX, we should map EISDIR to EPERM, but
        // we instead do what Linux does (and we must, as we use
        // the musl linux libc).
        throw new FS.ErrnoError(err);
      }
      if (!parent.node_ops.unlink) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      if (FS.isMountpoint(node)) {
        throw new FS.ErrnoError(ERRNO_CODES.EBUSY);
      }
      try {
        if (FS.trackingDelegate['willDeletePath']) {
          FS.trackingDelegate['willDeletePath'](path);
        }
      } catch(e) {
        console.log("FS.trackingDelegate['willDeletePath']('"+path+"') threw an exception: " + e.message);
      }
      parent.node_ops.unlink(parent, name);
      FS.destroyNode(node);
      try {
        if (FS.trackingDelegate['onDeletePath']) FS.trackingDelegate['onDeletePath'](path);
      } catch(e) {
        console.log("FS.trackingDelegate['onDeletePath']('"+path+"') threw an exception: " + e.message);
      }
    },
    readlink: function(path) {
      var lookup = FS.lookupPath(path);
      var link = lookup.node;
      if (!link) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      }
      if (!link.node_ops.readlink) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      return PATH.resolve(FS.getPath(link.parent), link.node_ops.readlink(link));
    },
    stat: function(path, dontFollow) {
      var lookup = FS.lookupPath(path, { follow: !dontFollow });
      var node = lookup.node;
      if (!node) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      }
      if (!node.node_ops.getattr) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      return node.node_ops.getattr(node);
    },
    lstat: function(path) {
      return FS.stat(path, true);
    },
    chmod: function(path, mode, dontFollow) {
      var node;
      if (typeof path === 'string') {
        var lookup = FS.lookupPath(path, { follow: !dontFollow });
        node = lookup.node;
      } else {
        node = path;
      }
      if (!node.node_ops.setattr) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      node.node_ops.setattr(node, {
        mode: (mode & {{{ cDefine('S_IALLUGO') }}}) | (node.mode & ~{{{ cDefine('S_IALLUGO') }}}),
        timestamp: Date.now()
      });
    },
    lchmod: function(path, mode) {
      FS.chmod(path, mode, true);
    },
    fchmod: function(fd, mode) {
      var stream = FS.getStream(fd);
      if (!stream) {
        throw new FS.ErrnoError(ERRNO_CODES.EBADF);
      }
      FS.chmod(stream.node, mode);
    },
    chown: function(path, uid, gid, dontFollow) {
      var node;
      if (typeof path === 'string') {
        var lookup = FS.lookupPath(path, { follow: !dontFollow });
        node = lookup.node;
      } else {
        node = path;
      }
      if (!node.node_ops.setattr) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      node.node_ops.setattr(node, {
        timestamp: Date.now()
        // we ignore the uid / gid for now
      });
    },
    lchown: function(path, uid, gid) {
      FS.chown(path, uid, gid, true);
    },
    fchown: function(fd, uid, gid) {
      var stream = FS.getStream(fd);
      if (!stream) {
        throw new FS.ErrnoError(ERRNO_CODES.EBADF);
      }
      FS.chown(stream.node, uid, gid);
    },
    truncate: function(path, len) {
      if (len < 0) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      var node;
      if (typeof path === 'string') {
        var lookup = FS.lookupPath(path, { follow: true });
        node = lookup.node;
      } else {
        node = path;
      }
      if (!node.node_ops.setattr) {
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      if (FS.isDir(node.mode)) {
        throw new FS.ErrnoError(ERRNO_CODES.EISDIR);
      }
      if (!FS.isFile(node.mode)) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      var err = FS.nodePermissions(node, 'w');
      if (err) {
        throw new FS.ErrnoError(err);
      }
      node.node_ops.setattr(node, {
        size: len,
        timestamp: Date.now()
      });
    },
    ftruncate: function(fd, len) {
      var stream = FS.getStream(fd);
      if (!stream) {
        throw new FS.ErrnoError(ERRNO_CODES.EBADF);
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_RDONLY')}}}) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      FS.truncate(stream.node, len);
    },
    utime: function(path, atime, mtime) {
      var lookup = FS.lookupPath(path, { follow: true });
      var node = lookup.node;
      node.node_ops.setattr(node, {
        timestamp: Math.max(atime, mtime)
      });
    },
    open: function(path, flags, mode, fd_start, fd_end) {
      if (path === "") {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      }
      flags = typeof flags === 'string' ? FS.modeStringToFlags(flags) : flags;
      mode = typeof mode === 'undefined' ? 438 /* 0666 */ : mode;
      if ((flags & {{{ cDefine('O_CREAT') }}})) {
        mode = (mode & {{{ cDefine('S_IALLUGO') }}}) | {{{ cDefine('S_IFREG') }}};
      } else {
        mode = 0;
      }
      var node;
      if (typeof path === 'object') {
        node = path;
      } else {
        path = PATH.normalize(path);
        try {
          var lookup = FS.lookupPath(path, {
            follow: !(flags & {{{ cDefine('O_NOFOLLOW') }}})
          });
          node = lookup.node;
        } catch (e) {
          // ignore
        }
      }
      // perhaps we need to create the node
      var created = false;
      if ((flags & {{{ cDefine('O_CREAT') }}})) {
        if (node) {
          // if O_CREAT and O_EXCL are set, error out if the node already exists
          if ((flags & {{{ cDefine('O_EXCL') }}})) {
            throw new FS.ErrnoError(ERRNO_CODES.EEXIST);
          }
        } else {
          // node doesn't exist, try to create it
          node = FS.mknod(path, mode, 0);
          created = true;
        }
      }
      if (!node) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      }
      // can't truncate a device
      if (FS.isChrdev(node.mode)) {
        flags &= ~{{{ cDefine('O_TRUNC') }}};
      }
      // if asked only for a directory, then this must be one
      if ((flags & {{{ cDefine('O_DIRECTORY') }}}) && !FS.isDir(node.mode)) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOTDIR);
      }
      // check permissions, if this is not a file we just created now (it is ok to
      // create and write to a file with read-only permissions; it is read-only
      // for later use)
      if (!created) {
        var err = FS.mayOpen(node, flags);
        if (err) {
          throw new FS.ErrnoError(err);
        }
      }
      // do truncation if necessary
      if ((flags & {{{ cDefine('O_TRUNC')}}})) {
        FS.truncate(node, 0);
      }
      // we've already handled these, don't pass down to the underlying vfs
      flags &= ~({{{ cDefine('O_EXCL') }}} | {{{ cDefine('O_TRUNC') }}});

      // register the stream with the filesystem
      var stream = FS.createStream({
        node: node,
        path: FS.getPath(node),  // we want the absolute path to the node
        flags: flags,
        seekable: true,
        position: 0,
        stream_ops: node.stream_ops,
        // used by the file family libc calls (fopen, fwrite, ferror, etc.)
        ungotten: [],
        error: false
      }, fd_start, fd_end);
      // call the new stream's open function
      if (stream.stream_ops.open) {
        stream.stream_ops.open(stream);
      }
      if (Module['logReadFiles'] && !(flags & {{{ cDefine('O_WRONLY')}}})) {
        if (!FS.readFiles) FS.readFiles = {};
        if (!(path in FS.readFiles)) {
          FS.readFiles[path] = 1;
          Module['printErr']('read file: ' + path);
        }
      }
      try {
        if (FS.trackingDelegate['onOpenFile']) {
          var trackingFlags = 0;
          if ((flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_WRONLY') }}}) {
            trackingFlags |= FS.tracking.openFlags.READ;
          }
          if ((flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_RDONLY') }}}) {
            trackingFlags |= FS.tracking.openFlags.WRITE;
          }
          FS.trackingDelegate['onOpenFile'](path, trackingFlags);
        }
      } catch(e) {
        console.log("FS.trackingDelegate['onOpenFile']('"+path+"', flags) threw an exception: " + e.message);
      }
      return stream;
    },
    close: function(stream) {
      if (stream.getdents) stream.getdents = null; // free readdir state
      try {
        if (stream.stream_ops.close) {
          stream.stream_ops.close(stream);
        }
      } catch (e) {
        throw e;
      } finally {
        FS.closeStream(stream.fd);
      }
    },
    llseek: function(stream, offset, whence) {
      if (!stream.seekable || !stream.stream_ops.llseek) {
        throw new FS.ErrnoError(ERRNO_CODES.ESPIPE);
      }
      stream.position = stream.stream_ops.llseek(stream, offset, whence);
      stream.ungotten = [];
      return stream.position;
    },
    read: function(stream, buffer, offset, length, position) {
      if (length < 0 || position < 0) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_WRONLY')}}}) {
        throw new FS.ErrnoError(ERRNO_CODES.EBADF);
      }
      if (FS.isDir(stream.node.mode)) {
        throw new FS.ErrnoError(ERRNO_CODES.EISDIR);
      }
      if (!stream.stream_ops.read) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      var seeking = true;
      if (typeof position === 'undefined') {
        position = stream.position;
        seeking = false;
      } else if (!stream.seekable) {
        throw new FS.ErrnoError(ERRNO_CODES.ESPIPE);
      }
      var bytesRead = stream.stream_ops.read(stream, buffer, offset, length, position);
      if (!seeking) stream.position += bytesRead;
      return bytesRead;
    },
    write: function(stream, buffer, offset, length, position, canOwn) {
      if (length < 0 || position < 0) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_RDONLY')}}}) {
        throw new FS.ErrnoError(ERRNO_CODES.EBADF);
      }
      if (FS.isDir(stream.node.mode)) {
        throw new FS.ErrnoError(ERRNO_CODES.EISDIR);
      }
      if (!stream.stream_ops.write) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      if (stream.flags & {{{ cDefine('O_APPEND') }}}) {
        // seek to the end before writing in append mode
        FS.llseek(stream, 0, {{{ cDefine('SEEK_END') }}});
      }
      var seeking = true;
      if (typeof position === 'undefined') {
        position = stream.position;
        seeking = false;
      } else if (!stream.seekable) {
        throw new FS.ErrnoError(ERRNO_CODES.ESPIPE);
      }
      var bytesWritten = stream.stream_ops.write(stream, buffer, offset, length, position, canOwn);
      if (!seeking) stream.position += bytesWritten;
      try {
        if (stream.path && FS.trackingDelegate['onWriteToFile']) FS.trackingDelegate['onWriteToFile'](stream.path);
      } catch(e) {
        console.log("FS.trackingDelegate['onWriteToFile']('"+path+"') threw an exception: " + e.message);
      }
      return bytesWritten;
    },
    allocate: function(stream, offset, length) {
      if (offset < 0 || length <= 0) {
        throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_RDONLY')}}}) {
        throw new FS.ErrnoError(ERRNO_CODES.EBADF);
      }
      if (!FS.isFile(stream.node.mode) && !FS.isDir(stream.node.mode)) {
        throw new FS.ErrnoError(ERRNO_CODES.ENODEV);
      }
      if (!stream.stream_ops.allocate) {
        throw new FS.ErrnoError(ERRNO_CODES.EOPNOTSUPP);
      }
      stream.stream_ops.allocate(stream, offset, length);
    },
    mmap: function(stream, buffer, offset, length, position, prot, flags) {
      // TODO if PROT is PROT_WRITE, make sure we have write access
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_WRONLY')}}}) {
        throw new FS.ErrnoError(ERRNO_CODES.EACCES);
      }
      if (!stream.stream_ops.mmap) {
        throw new FS.ErrnoError(ERRNO_CODES.ENODEV);
      }
      return stream.stream_ops.mmap(stream, buffer, offset, length, position, prot, flags);
    },
    msync: function(stream, buffer, offset, length, mmapFlags) {
      if (!stream || !stream.stream_ops.msync) {
        return 0;
      }
      return stream.stream_ops.msync(stream, buffer, offset, length, mmapFlags);
    },
    munmap: function(stream) {
      return 0;
    },
    ioctl: function(stream, cmd, arg) {
      if (!stream.stream_ops.ioctl) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOTTY);
      }
      return stream.stream_ops.ioctl(stream, cmd, arg);
    },
    readFile: function(path, opts) {
      opts = opts || {};
      opts.flags = opts.flags || 'r';
      opts.encoding = opts.encoding || 'binary';
      if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
        throw new Error('Invalid encoding type "' + opts.encoding + '"');
      }
      var ret;
      var stream = FS.open(path, opts.flags);
      var stat = FS.stat(path);
      var length = stat.size;
      var buf = new Uint8Array(length);
      FS.read(stream, buf, 0, length, 0);
      if (opts.encoding === 'utf8') {
        ret = UTF8ArrayToString(buf, 0);
      } else if (opts.encoding === 'binary') {
        ret = buf;
      }
      FS.close(stream);
      return ret;
    },
    writeFile: function(path, data, opts) {
      opts = opts || {};
      opts.flags = opts.flags || 'w';
      opts.encoding = opts.encoding || 'utf8';
      if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
        throw new Error('Invalid encoding type "' + opts.encoding + '"');
      }
      var stream = FS.open(path, opts.flags, opts.mode);
      if (opts.encoding === 'utf8') {
        var buf = new Uint8Array(lengthBytesUTF8(data)+1);
        var actualNumBytes = stringToUTF8Array(data, buf, 0, buf.length);
        FS.write(stream, buf, 0, actualNumBytes, 0, opts.canOwn);
      } else if (opts.encoding === 'binary') {
        FS.write(stream, data, 0, data.length, 0, opts.canOwn);
      }
      FS.close(stream);
    },

    //
    // module-level FS code
    //
    cwd: function() {
      return FS.currentPath;
    },
    chdir: function(path) {
      var lookup = FS.lookupPath(path, { follow: true });
      if (lookup.node === null) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      }
      if (!FS.isDir(lookup.node.mode)) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOTDIR);
      }
      var err = FS.nodePermissions(lookup.node, 'x');
      if (err) {
        throw new FS.ErrnoError(err);
      }
      FS.currentPath = lookup.path;
    },
    createDefaultDirectories: function() {
      FS.mkdir('/tmp');
      FS.mkdir('/home');
      FS.mkdir('/home/web_user');
    },
    createDefaultDevices: function() {
      // create /dev
      FS.mkdir('/dev');
      // setup /dev/null
      FS.registerDevice(FS.makedev(1, 3), {
        read: function() { return 0; },
        write: function(stream, buffer, offset, length, pos) { return length; }
      });
      FS.mkdev('/dev/null', FS.makedev(1, 3));
      // setup /dev/tty and /dev/tty1
      // stderr needs to print output using Module['printErr']
      // so we register a second tty just for it.
      TTY.register(FS.makedev(5, 0), TTY.default_tty_ops);
      TTY.register(FS.makedev(6, 0), TTY.default_tty1_ops);
      FS.mkdev('/dev/tty', FS.makedev(5, 0));
      FS.mkdev('/dev/tty1', FS.makedev(6, 0));
      // setup /dev/[u]random
      var random_device;
      if (typeof crypto !== 'undefined') {
        // for modern web browsers
        var randomBuffer = new Uint8Array(1);
        random_device = function() { crypto.getRandomValues(randomBuffer); return randomBuffer[0]; };
      } else if (ENVIRONMENT_IS_NODE) {
        // for nodejs
        random_device = function() { return require('crypto').randomBytes(1)[0]; };
      } else {
        // default for ES5 platforms
        random_device = function() { return (Math.random()*256)|0; };
      }
      FS.createDevice('/dev', 'random', random_device);
      FS.createDevice('/dev', 'urandom', random_device);
      // we're not going to emulate the actual shm device,
      // just create the tmp dirs that reside in it commonly
      FS.mkdir('/dev/shm');
      FS.mkdir('/dev/shm/tmp');
    },
    createSpecialDirectories: function() {
      // create /proc/self/fd which allows /proc/self/fd/6 => readlink gives the name of the stream for fd 6 (see test_unistd_ttyname)
      FS.mkdir('/proc');
      FS.mkdir('/proc/self');
      FS.mkdir('/proc/self/fd');
      FS.mount({
        mount: function() {
          var node = FS.createNode('/proc/self', 'fd', {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */, {{{ cDefine('S_IXUGO') }}});
          node.node_ops = {
            lookup: function(parent, name) {
              var fd = +name;
              var stream = FS.getStream(fd);
              if (!stream) throw new FS.ErrnoError(ERRNO_CODES.EBADF);
              var ret = {
                parent: null,
                mount: { mountpoint: 'fake' },
                node_ops: { readlink: function() { return stream.path } }
              };
              ret.parent = ret; // make it look like a simple root node
              return ret;
            }
          };
          return node;
        }
      }, {}, '/proc/self/fd');
    },
    createStandardStreams: function() {
      // TODO deprecate the old functionality of a single
      // input / output callback and that utilizes FS.createDevice
      // and instead require a unique set of stream ops

      // by default, we symlink the standard streams to the
      // default tty devices. however, if the standard streams
      // have been overwritten we create a unique device for
      // them instead.
      if (Module['stdin']) {
        FS.createDevice('/dev', 'stdin', Module['stdin']);
      } else {
        FS.symlink('/dev/tty', '/dev/stdin');
      }
      if (Module['stdout']) {
        FS.createDevice('/dev', 'stdout', null, Module['stdout']);
      } else {
        FS.symlink('/dev/tty', '/dev/stdout');
      }
      if (Module['stderr']) {
        FS.createDevice('/dev', 'stderr', null, Module['stderr']);
      } else {
        FS.symlink('/dev/tty1', '/dev/stderr');
      }

      // open default streams for the stdin, stdout and stderr devices
      var stdin = FS.open('/dev/stdin', 'r');
      assert(stdin.fd === 0, 'invalid handle for stdin (' + stdin.fd + ')');

      var stdout = FS.open('/dev/stdout', 'w');
      assert(stdout.fd === 1, 'invalid handle for stdout (' + stdout.fd + ')');

      var stderr = FS.open('/dev/stderr', 'w');
      assert(stderr.fd === 2, 'invalid handle for stderr (' + stderr.fd + ')');
    },
    ensureErrnoError: function() {
      if (FS.ErrnoError) return;
      FS.ErrnoError = function ErrnoError(errno, node) {
        //Module.printErr(stackTrace()); // useful for debugging
        this.node = node;
        this.setErrno = function(errno) {
          this.errno = errno;
          for (var key in ERRNO_CODES) {
            if (ERRNO_CODES[key] === errno) {
              this.code = key;
              break;
            }
          }
        };
        this.setErrno(errno);
        this.message = ERRNO_MESSAGES[errno];
#if ASSERTIONS
        if (this.stack) this.stack = demangleAll(this.stack);
#endif
      };
      FS.ErrnoError.prototype = new Error();
      FS.ErrnoError.prototype.constructor = FS.ErrnoError;
      // Some errors may happen quite a bit, to avoid overhead we reuse them (and suffer a lack of stack info)
      [ERRNO_CODES.ENOENT].forEach(function(code) {
        FS.genericErrors[code] = new FS.ErrnoError(code);
        FS.genericErrors[code].stack = '<generic error, no stack>';
      });
    },
    staticInit: function() {
      FS.ensureErrnoError();

      FS.nameTable = new Array(4096);

      FS.mount(MEMFS, {}, '/');

      FS.createDefaultDirectories();
      FS.createDefaultDevices();
      FS.createSpecialDirectories();

      FS.filesystems = {
        'MEMFS': MEMFS,
#if __EMSCRIPTEN_HAS_idbfs_js__
        'IDBFS': IDBFS,
#endif
#if __EMSCRIPTEN_HAS_nodefs_js__
        'NODEFS': NODEFS,
#endif
#if __EMSCRIPTEN_HAS_workerfs_js__
        'WORKERFS': WORKERFS,
#endif
      };
    },
    init: function(input, output, error) {
      assert(!FS.init.initialized, 'FS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)');
      FS.init.initialized = true;

      FS.ensureErrnoError();

      // Allow Module.stdin etc. to provide defaults, if none explicitly passed to us here
      Module['stdin'] = input || Module['stdin'];
      Module['stdout'] = output || Module['stdout'];
      Module['stderr'] = error || Module['stderr'];

      FS.createStandardStreams();
    },
    quit: function() {
      FS.init.initialized = false;
      // force-flush all streams, so we get musl std streams printed out
      var fflush = Module['_fflush'];
      if (fflush) fflush(0);
      // close all of our streams
      for (var i = 0; i < FS.streams.length; i++) {
        var stream = FS.streams[i];
        if (!stream) {
          continue;
        }
        FS.close(stream);
      }
    },

    //
    // old v1 compatibility functions
    //
    getMode: function(canRead, canWrite) {
      var mode = 0;
      if (canRead) mode |= {{{ cDefine('S_IRUGO') }}} | {{{ cDefine('S_IXUGO') }}};
      if (canWrite) mode |= {{{ cDefine('S_IWUGO') }}};
      return mode;
    },
    joinPath: function(parts, forceRelative) {
      var path = PATH.join.apply(null, parts);
      if (forceRelative && path[0] == '/') path = path.substr(1);
      return path;
    },
    absolutePath: function(relative, base) {
      return PATH.resolve(base, relative);
    },
    standardizePath: function(path) {
      return PATH.normalize(path);
    },
    findObject: function(path, dontResolveLastLink) {
      var ret = FS.analyzePath(path, dontResolveLastLink);
      if (ret.exists) {
        return ret.object;
      } else {
        ___setErrNo(ret.error);
        return null;
      }
    },
    analyzePath: function(path, dontResolveLastLink) {
      // operate from within the context of the symlink's target
      try {
        var lookup = FS.lookupPath(path, { follow: !dontResolveLastLink });
        path = lookup.path;
      } catch (e) {
      }
      var ret = {
        isRoot: false, exists: false, error: 0, name: null, path: null, object: null,
        parentExists: false, parentPath: null, parentObject: null
      };
      try {
        var lookup = FS.lookupPath(path, { parent: true });
        ret.parentExists = true;
        ret.parentPath = lookup.path;
        ret.parentObject = lookup.node;
        ret.name = PATH.basename(path);
        lookup = FS.lookupPath(path, { follow: !dontResolveLastLink });
        ret.exists = true;
        ret.path = lookup.path;
        ret.object = lookup.node;
        ret.name = lookup.node.name;
        ret.isRoot = lookup.path === '/';
      } catch (e) {
        ret.error = e.errno;
      };
      return ret;
    },
    createFolder: function(parent, name, canRead, canWrite) {
      var path = PATH.join2(typeof parent === 'string' ? parent : FS.getPath(parent), name);
      var mode = FS.getMode(canRead, canWrite);
      return FS.mkdir(path, mode);
    },
    createPath: function(parent, path, canRead, canWrite) {
      parent = typeof parent === 'string' ? parent : FS.getPath(parent);
      var parts = path.split('/').reverse();
      while (parts.length) {
        var part = parts.pop();
        if (!part) continue;
        var current = PATH.join2(parent, part);
        try {
          FS.mkdir(current);
        } catch (e) {
          // ignore EEXIST
        }
        parent = current;
      }
      return current;
    },
    createFile: function(parent, name, properties, canRead, canWrite) {
      var path = PATH.join2(typeof parent === 'string' ? parent : FS.getPath(parent), name);
      var mode = FS.getMode(canRead, canWrite);
      return FS.create(path, mode);
    },
    createDataFile: function(parent, name, data, canRead, canWrite, canOwn) {
      var path = name ? PATH.join2(typeof parent === 'string' ? parent : FS.getPath(parent), name) : parent;
      var mode = FS.getMode(canRead, canWrite);
      var node = FS.create(path, mode);
      if (data) {
        if (typeof data === 'string') {
          var arr = new Array(data.length);
          for (var i = 0, len = data.length; i < len; ++i) arr[i] = data.charCodeAt(i);
          data = arr;
        }
        // make sure we can write to the file
        FS.chmod(node, mode | {{{ cDefine('S_IWUGO') }}});
        var stream = FS.open(node, 'w');
        FS.write(stream, data, 0, data.length, 0, canOwn);
        FS.close(stream);
        FS.chmod(node, mode);
      }
      return node;
    },
    createDevice: function(parent, name, input, output) {
      var path = PATH.join2(typeof parent === 'string' ? parent : FS.getPath(parent), name);
      var mode = FS.getMode(!!input, !!output);
      if (!FS.createDevice.major) FS.createDevice.major = 64;
      var dev = FS.makedev(FS.createDevice.major++, 0);
      // Create a fake device that a set of stream ops to emulate
      // the old behavior.
      FS.registerDevice(dev, {
        open: function(stream) {
          stream.seekable = false;
        },
        close: function(stream) {
          // flush any pending line data
          if (output && output.buffer && output.buffer.length) {
            output({{{ charCode('\n') }}});
          }
        },
        read: function(stream, buffer, offset, length, pos /* ignored */) {
          var bytesRead = 0;
          for (var i = 0; i < length; i++) {
            var result;
            try {
              result = input();
            } catch (e) {
              throw new FS.ErrnoError(ERRNO_CODES.EIO);
            }
            if (result === undefined && bytesRead === 0) {
              throw new FS.ErrnoError(ERRNO_CODES.EAGAIN);
            }
            if (result === null || result === undefined) break;
            bytesRead++;
            buffer[offset+i] = result;
          }
          if (bytesRead) {
            stream.node.timestamp = Date.now();
          }
          return bytesRead;
        },
        write: function(stream, buffer, offset, length, pos) {
          for (var i = 0; i < length; i++) {
            try {
              output(buffer[offset+i]);
            } catch (e) {
              throw new FS.ErrnoError(ERRNO_CODES.EIO);
            }
          }
          if (length) {
            stream.node.timestamp = Date.now();
          }
          return i;
        }
      });
      return FS.mkdev(path, mode, dev);
    },
    createLink: function(parent, name, target, canRead, canWrite) {
      var path = PATH.join2(typeof parent === 'string' ? parent : FS.getPath(parent), name);
      return FS.symlink(target, path);
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
          obj.usedBytes = obj.contents.length;
        } catch (e) {
          success = false;
        }
      } else {
        throw new Error('Cannot load without read() or XMLHttpRequest.');
      }
      if (!success) ___setErrNo(ERRNO_CODES.EIO);
      return success;
    },
    // Creates a file record for lazy-loading from a URL. XXX This requires a synchronous
    // XHR, which is not possible in browsers except in a web worker! Use preloading,
    // either --preload-file in emcc or FS.createPreloadedFile
    createLazyFile: function(parent, name, url, canRead, canWrite) {
      // Lazy chunked Uint8Array (implements get and length from Uint8Array). Actual getting is abstracted away for eventual reuse.
      function LazyUint8Array() {
        this.lengthKnown = false;
        this.chunks = []; // Loaded chunks. Index is the chunk number
      }
      LazyUint8Array.prototype.get = function LazyUint8Array_get(idx) {
        if (idx > this.length-1 || idx < 0) {
          return undefined;
        }
        var chunkOffset = idx % this.chunkSize;
        var chunkNum = (idx / this.chunkSize)|0;
        return this.getter(chunkNum)[chunkOffset];
      }
      LazyUint8Array.prototype.setDataGetter = function LazyUint8Array_setDataGetter(getter) {
        this.getter = getter;
      }
      LazyUint8Array.prototype.cacheLength = function LazyUint8Array_cacheLength() {
        // Find length
        var xhr = new XMLHttpRequest();
        xhr.open('HEAD', url, false);
        xhr.send(null);
        if (!(xhr.status >= 200 && xhr.status < 300 || xhr.status === 304)) throw new Error("Couldn't load " + url + ". Status: " + xhr.status);
        var datalength = Number(xhr.getResponseHeader("Content-length"));
        var header;
        var hasByteServing = (header = xhr.getResponseHeader("Accept-Ranges")) && header === "bytes";
        var usesGzip = (header = xhr.getResponseHeader("Content-Encoding")) && header === "gzip";

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
        var lazyArray = this;
        lazyArray.setDataGetter(function(chunkNum) {
          var start = chunkNum * chunkSize;
          var end = (chunkNum+1) * chunkSize - 1; // including this byte
          end = Math.min(end, datalength-1); // if datalength-1 is selected, this is the last block
          if (typeof(lazyArray.chunks[chunkNum]) === "undefined") {
            lazyArray.chunks[chunkNum] = doXHR(start, end);
          }
          if (typeof(lazyArray.chunks[chunkNum]) === "undefined") throw new Error("doXHR failed!");
          return lazyArray.chunks[chunkNum];
        });

        if (usesGzip || !datalength) {
          // if the server uses gzip or doesn't supply the length, we have to download the whole file to get the (uncompressed) length
          chunkSize = datalength = 1; // this will force getter(0)/doXHR do download the whole file
          datalength = this.getter(0).length;
          chunkSize = datalength;
          console.log("LazyFiles on gzip forces download of the whole file when length is accessed");
        }

        this._length = datalength;
        this._chunkSize = chunkSize;
        this.lengthKnown = true;
      }
      if (typeof XMLHttpRequest !== 'undefined') {
        if (!ENVIRONMENT_IS_WORKER) throw 'Cannot do synchronous binary XHRs outside webworkers in modern browsers. Use --embed-file or --preload-file in emcc';
        var lazyArray = new LazyUint8Array();
        Object.defineProperties(lazyArray, {
          length: {
            get: function() {
              if(!this.lengthKnown) {
                this.cacheLength();
              }
              return this._length;
            }
          },
          chunkSize: {
            get: function() {
              if(!this.lengthKnown) {
                this.cacheLength();
              }
              return this._chunkSize;
            }
          }
        });

        var properties = { isDevice: false, contents: lazyArray };
      } else {
        var properties = { isDevice: false, url: url };
      }

      var node = FS.createFile(parent, name, properties, canRead, canWrite);
      // This is a total hack, but I want to get this lazy file code out of the
      // core of MEMFS. If we want to keep this lazy file concept I feel it should
      // be its own thin LAZYFS proxying calls to MEMFS.
      if (properties.contents) {
        node.contents = properties.contents;
      } else if (properties.url) {
        node.contents = null;
        node.url = properties.url;
      }
      // Add a function that defers querying the file size until it is asked the first time.
      Object.defineProperties(node, {
        usedBytes: {
          get: function() { return this.contents.length; }
        }
      });
      // override each stream op with one that tries to force load the lazy file first
      var stream_ops = {};
      var keys = Object.keys(node.stream_ops);
      keys.forEach(function(key) {
        var fn = node.stream_ops[key];
        stream_ops[key] = function forceLoadLazyFile() {
          if (!FS.forceLoadFile(node)) {
            throw new FS.ErrnoError(ERRNO_CODES.EIO);
          }
          return fn.apply(null, arguments);
        };
      });
      // use a custom read function
      stream_ops.read = function stream_ops_read(stream, buffer, offset, length, position) {
        if (!FS.forceLoadFile(node)) {
          throw new FS.ErrnoError(ERRNO_CODES.EIO);
        }
        var contents = stream.node.contents;
        if (position >= contents.length)
          return 0;
        var size = Math.min(contents.length - position, length);
        assert(size >= 0);
        if (contents.slice) { // normal array
          for (var i = 0; i < size; i++) {
            buffer[offset + i] = contents[position + i];
          }
        } else {
          for (var i = 0; i < size; i++) { // LazyUint8Array from sync binary XHR
            buffer[offset + i] = contents.get(position + i);
          }
        }
        return size;
      };
      node.stream_ops = stream_ops;
      return node;
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
    createPreloadedFile: function(parent, name, url, canRead, canWrite, onload, onerror, dontCreateFile, canOwn, preFinish) {
      Browser.init(); // XXX perhaps this method should move onto Browser?
      // TODO we should allow people to just pass in a complete filename instead
      // of parent and name being that we just join them anyways
      var fullname = name ? PATH.resolve(PATH.join2(parent, name)) : parent;
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
        Browser.asyncLoad(url, function(byteArray) {
          processData(byteArray);
        }, onerror);
      } else {
        processData(url);
      }
    },

    //
    // persistence
    //
    indexedDB: function() {
      return window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
    },

    DB_NAME: function() {
      return 'EM_FS_' + window.location.pathname;
    },
    DB_VERSION: 20,
    DB_STORE_NAME: 'FILE_DATA',

    // asynchronously saves a list of files to an IndexedDB. The DB will be created if not already existing.
    saveFilesToDB: function(paths, onload, onerror) {
      onload = onload || function(){};
      onerror = onerror || function(){};
      var indexedDB = FS.indexedDB();
      try {
        var openRequest = indexedDB.open(FS.DB_NAME(), FS.DB_VERSION);
      } catch (e) {
        return onerror(e);
      }
      openRequest.onupgradeneeded = function openRequest_onupgradeneeded() {
        console.log('creating db');
        var db = openRequest.result;
        db.createObjectStore(FS.DB_STORE_NAME);
      };
      openRequest.onsuccess = function openRequest_onsuccess() {
        var db = openRequest.result;
        var transaction = db.transaction([FS.DB_STORE_NAME], 'readwrite');
        var files = transaction.objectStore(FS.DB_STORE_NAME);
        var ok = 0, fail = 0, total = paths.length;
        function finish() {
          if (fail == 0) onload(); else onerror();
        }
        paths.forEach(function(path) {
          var putRequest = files.put(FS.analyzePath(path).object.contents, path);
          putRequest.onsuccess = function putRequest_onsuccess() { ok++; if (ok + fail == total) finish() };
          putRequest.onerror = function putRequest_onerror() { fail++; if (ok + fail == total) finish() };
        });
        transaction.onerror = onerror;
      };
      openRequest.onerror = onerror;
    },

    // asynchronously loads a file from IndexedDB.
    loadFilesFromDB: function(paths, onload, onerror) {
      onload = onload || function(){};
      onerror = onerror || function(){};
      var indexedDB = FS.indexedDB();
      try {
        var openRequest = indexedDB.open(FS.DB_NAME(), FS.DB_VERSION);
      } catch (e) {
        return onerror(e);
      }
      openRequest.onupgradeneeded = onerror; // no database to load from
      openRequest.onsuccess = function openRequest_onsuccess() {
        var db = openRequest.result;
        try {
          var transaction = db.transaction([FS.DB_STORE_NAME], 'readonly');
        } catch(e) {
          onerror(e);
          return;
        }
        var files = transaction.objectStore(FS.DB_STORE_NAME);
        var ok = 0, fail = 0, total = paths.length;
        function finish() {
          if (fail == 0) onload(); else onerror();
        }
        paths.forEach(function(path) {
          var getRequest = files.get(path);
          getRequest.onsuccess = function getRequest_onsuccess() {
            if (FS.analyzePath(path).exists) {
              FS.unlink(path);
            }
            FS.createDataFile(PATH.dirname(path), PATH.basename(path), getRequest.result, true, true, true);
            ok++;
            if (ok + fail == total) finish();
          };
          getRequest.onerror = function getRequest_onerror() { fail++; if (ok + fail == total) finish() };
        });
        transaction.onerror = onerror;
      };
      openRequest.onerror = onerror;
    }
  }
});

