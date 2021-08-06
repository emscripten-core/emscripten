/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

 mergeInto(LibraryManager.library, {
  $PThreadFS__deps: ['$getRandomDevice', '$PATH', '$PATH_FS', '$TTY_ASYNC', '$MEMFS_ASYNC', 
#if ASSERTIONS
    '$ERRNO_MESSAGES', '$ERRNO_CODES',
#endif
    ],
  $PThreadFS: {
    root: null,
    mounts: [],
    devices: {},
    streams: [],
    nextInode: 1,
    nameTable: null,
    currentPath: '/',
    initialized: false,
    // Whether we are currently ignoring permissions. Useful when preparing the
    // filesystem and creating files inside read-only folders.
    // This is set to false during `preInit`, allowing you to modify the
    // filesystem freely up until that point (e.g. during `preRun`).
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

    //
    // paths
    //
    lookupPath: async function(path, opts) {
      path = PATH_FS.resolve(PThreadFS.cwd(), path);
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
        throw new PThreadFS.ErrnoError({{{ cDefine('ELOOP') }}});
      }

      // split the path
      var parts = PATH.normalizeArray(path.split('/').filter(function(p) {
        return !!p;
      }), false);

      // start at the root
      var current = PThreadFS.root;
      var current_path = '/';

      for (var i = 0; i < parts.length; i++) {
        var islast = (i === parts.length-1);
        if (islast && opts.parent) {
          // stop resolving
          break;
        }

        current = await PThreadFS.lookupNode(current, parts[i]);
        current_path = PATH.join2(current_path, parts[i]);

        // jump to the mount's root node if this is a mountpoint
        if (PThreadFS.isMountpoint(current)) {
          if (!islast || (islast && opts.follow_mount)) {
            current = current.mounted.root;
          }
        }

        // by default, lookupPath will not follow a symlink if it is the final path component.
        // setting opts.follow = true will override this behavior.
        if (!islast || opts.follow) {
          var count = 0;
          while (PThreadFS.isLink(current.mode)) {
            var link = await PThreadFS.readlink(current_path);
            current_path = PATH_FS.resolve(PATH.dirname(current_path), link);

            var lookup = await PThreadFS.lookupPath(current_path, { recurse_count: opts.recurse_count });
            current = lookup.node;

            if (count++ > 40) {  // limit max consecutive symlinks to 40 (SYMLOOP_MAX).
              throw new PThreadFS.ErrnoError({{{ cDefine('ELOOP') }}});
            }
          }
        }
      }

      return { path: current_path, node: current };
    },
    getPath: function(node) {
      var path;
      while (true) {
        if (PThreadFS.isRoot(node)) {
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
      return ((parentid + hash) >>> 0) % PThreadFS.nameTable.length;
    },
    hashAddNode: function(node) {
      var hash = PThreadFS.hashName(node.parent.id, node.name);
      node.name_next = PThreadFS.nameTable[hash];
      PThreadFS.nameTable[hash] = node;
    },
    hashRemoveNode: function(node) {
      var hash = PThreadFS.hashName(node.parent.id, node.name);
      if (PThreadFS.nameTable[hash] === node) {
        PThreadFS.nameTable[hash] = node.name_next;
      } else {
        var current = PThreadFS.nameTable[hash];
        while (current) {
          if (current.name_next === node) {
            current.name_next = node.name_next;
            break;
          }
          current = current.name_next;
        }
      }
    },
    lookupNode: async function(parent, name) {
      var errCode = PThreadFS.mayLookup(parent);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode, parent);
      }
      var hash = PThreadFS.hashName(parent.id, name);
#if CASE_INSENSITIVE_FS
      name = name.toLowerCase();
#endif
      for (var node = PThreadFS.nameTable[hash]; node; node = node.name_next) {
        var nodeName = node.name;
#if CASE_INSENSITIVE_FS
        nodeName = nodeName.toLowerCase();
#endif
        if (node.parent.id === parent.id && nodeName === name) {
          return node;
        }
      }
      // if we failed to find it in the cache, call into the VFS
      return await PThreadFS.lookup(parent, name);
    },
    createNode: function(parent, name, mode, rdev) {
#if ASSERTIONS
      assert(typeof parent === 'object')
#endif
      var node = new PThreadFS.FSNode(parent, name, mode, rdev);

      PThreadFS.hashAddNode(node);

      return node;
    },
    destroyNode: function(node) {
      PThreadFS.hashRemoveNode(node);
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
      // Extra quotes used here on the keys to this object otherwise jsifier will
      // erase them in the process of reading and then writing the JS library
      // code.
      '"r"': {{{ cDefine('O_RDONLY') }}},
      '"r+"': {{{ cDefine('O_RDWR') }}},
      '"w"': {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_WRONLY') }}},
      '"w+"': {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_RDWR') }}},
      '"a"': {{{ cDefine('O_APPEND') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_WRONLY') }}},
      '"a+"': {{{ cDefine('O_APPEND') }}} | {{{ cDefine('O_CREAT') }}} | {{{ cDefine('O_RDWR') }}},
    },
    // convert the 'r', 'r+', etc. to it's corresponding set of O_* flags
    modeStringToFlags: function(str) {
      var flags = PThreadFS.flagModes[str];
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
      if (PThreadFS.ignorePermissions) {
        return 0;
      }
      // return 0 if any user, group or owner bits are set.
      if (perms.includes('r') && !(node.mode & {{{ cDefine('S_IRUGO') }}})) {
        return {{{ cDefine('EACCES') }}};
      } else if (perms.includes('w') && !(node.mode & {{{ cDefine('S_IWUGO') }}})) {
        return {{{ cDefine('EACCES') }}};
      } else if (perms.includes('x') && !(node.mode & {{{ cDefine('S_IXUGO') }}})) {
        return {{{ cDefine('EACCES') }}};
      }
      return 0;
    },
    mayLookup: function(dir) {
      var errCode = PThreadFS.nodePermissions(dir, 'x');
      if (errCode) return errCode;
      if (!dir.node_ops.lookup) return {{{ cDefine('EACCES') }}};
      return 0;
    },
    mayCreate: async function(dir, name) {
      try {
        var node = await PThreadFS.lookupNode(dir, name);
        return {{{ cDefine('EEXIST') }}};
      } catch (e) {
      }
      return PThreadFS.nodePermissions(dir, 'wx');
    },
    mayDelete: async function(dir, name, isdir) {
      var node;
      try {
        node = await PThreadFS.lookupNode(dir, name);
      } catch (e) {
        return e.errno;
      }
      var errCode = PThreadFS.nodePermissions(dir, 'wx');
      if (errCode) {
        return errCode;
      }
      if (isdir) {
        if (!PThreadFS.isDir(node.mode)) {
          return {{{ cDefine('ENOTDIR') }}};
        }
        if (PThreadFS.isRoot(node) || PThreadFS.getPath(node) === PThreadFS.cwd()) {
          return {{{ cDefine('EBUSY') }}};
        }
      } else {
        if (PThreadFS.isDir(node.mode)) {
          return {{{ cDefine('EISDIR') }}};
        }
      }
      return 0;
    },
    mayOpen: function(node, flags) {
      if (!node) {
        return {{{ cDefine('ENOENT') }}};
      }
      if (PThreadFS.isLink(node.mode)) {
        return {{{ cDefine('ELOOP') }}};
      } else if (PThreadFS.isDir(node.mode)) {
        if (PThreadFS.flagsToPermissionString(flags) !== 'r' || // opening for write
            (flags & {{{ cDefine('O_TRUNC') }}})) { // TODO: check for O_SEARCH? (== search for dir only)
          return {{{ cDefine('EISDIR') }}};
        }
      }
      return PThreadFS.nodePermissions(node, PThreadFS.flagsToPermissionString(flags));
    },

    //
    // streams
    //
    MAX_OPEN_FDS: 4096,
    nextfd: function(fd_start, fd_end) {
      fd_start = fd_start || 0;
      fd_end = fd_end || PThreadFS.MAX_OPEN_FDS;
      for (var fd = fd_start; fd <= fd_end; fd++) {
        if (!PThreadFS.streams[fd]) {
          return fd;
        }
      }
      throw new PThreadFS.ErrnoError({{{ cDefine('EMFILE') }}});
    },
    getStream: function(fd) {
      return PThreadFS.streams[fd];
    },
    // TODO parameterize this function such that a stream
    // object isn't directly passed in. not possible until
    // SOCKFS is completed.
    createStream: function(stream, fd_start, fd_end) {
      if (!PThreadFS.FSStream) {
        PThreadFS.FSStream = /** @constructor */ function(){};
        PThreadFS.FSStream.prototype = {
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
        };
      }
      // clone it, so we can return an instance of FSStream
      var newStream = new PThreadFS.FSStream();
      for (var p in stream) {
        newStream[p] = stream[p];
      }
      stream = newStream;
      var fd = PThreadFS.nextfd(fd_start, fd_end);
      stream.fd = fd;
      PThreadFS.streams[fd] = stream;
      return stream;
    },
    closeStream: function(fd) {
      PThreadFS.streams[fd] = null;
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
        var device = PThreadFS.getDevice(stream.node.rdev);
        // override node's stream ops with the device's
        stream.stream_ops = device.stream_ops;
        // forward the open call
        if (stream.stream_ops.open) {
          stream.stream_ops.open(stream);
        }
      },
      llseek: function() {
        throw new PThreadFS.ErrnoError({{{ cDefine('ESPIPE') }}});
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
      PThreadFS.devices[dev] = { stream_ops: ops };
    },
    getDevice: function(dev) {
      return PThreadFS.devices[dev];
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

      PThreadFS.syncFSRequests++;

      if (PThreadFS.syncFSRequests > 1) {
        err('warning: ' + PThreadFS.syncFSRequests + ' PThreadFS.syncfs operations in flight at once, probably just doing extra work');
      }

      var mounts = PThreadFS.getMounts(PThreadFS.root.mount);
      var completed = 0;

      function doCallback(errCode) {
#if ASSERTIONS
        assert(PThreadFS.syncFSRequests > 0);
#endif
        PThreadFS.syncFSRequests--;
        return callback(errCode);
      }

      function done(errCode) {
        if (errCode) {
          if (!done.errored) {
            done.errored = true;
            return doCallback(errCode);
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
    mount: async function(type, opts, mountpoint) {
#if ASSERTIONS
      if (typeof type === 'string') {
        // The filesystem was not included, and instead we have an error
        // message stored in the variable.
        throw type;
      }
#endif
      var root = mountpoint === '/';
      var pseudo = !mountpoint;
      var node;

      if (root && PThreadFS.root) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBUSY') }}});
      } else if (!root && !pseudo) {
        var lookup = await PThreadFS.lookupPath(mountpoint, { follow_mount: false });

        mountpoint = lookup.path;  // use the absolute path
        node = lookup.node;

        if (PThreadFS.isMountpoint(node)) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EBUSY') }}});
        }

        if (!PThreadFS.isDir(node.mode)) {
          throw new PThreadFS.ErrnoError({{{ cDefine('ENOTDIR') }}});
        }
      }

      var mount = {
        type: type,
        opts: opts,
        mountpoint: mountpoint,
        mounts: []
      };

      // create a root node for the fs
      var mountRoot = await type.mount(mount);
      mountRoot.mount = mount;
      mount.root = mountRoot;

      if (root) {
        PThreadFS.root = mountRoot;
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
    unmount: async function (mountpoint) {
      var lookup = await PThreadFS.lookupPath(mountpoint, { follow_mount: false });

      if (!PThreadFS.isMountpoint(lookup.node)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }

      // destroy the nodes for this mount, and all its child mounts
      var node = lookup.node;
      var mount = node.mounted;
      var mounts = PThreadFS.getMounts(mount);

      Object.keys(PThreadFS.nameTable).forEach(function (hash) {
        var current = PThreadFS.nameTable[hash];

        while (current) {
          var next = current.name_next;

          if (mounts.includes(current.mount)) {
            PThreadFS.destroyNode(current);
          }

          current = next;
        }
      });

      // no longer a mountpoint
      node.mounted = null;

      // remove this mount from the child mounts
      var idx = node.mount.mounts.indexOf(mount);
#if ASSERTIONS
      assert(idx !== -1);
#endif
      node.mount.mounts.splice(idx, 1);
    },
    lookup: async function(parent, name) {
      return await parent.node_ops.lookup(parent, name);
    },
    // generic function for all node creation
    mknod: async function(path, mode, dev) {
      var lookup = await PThreadFS.lookupPath(path, { parent: true });
      var parent = lookup.node;
      var name = PATH.basename(path);
      if (!name || name === '.' || name === '..') {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var errCode = await PThreadFS.mayCreate(parent, name);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      if (!parent.node_ops.mknod) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      return await parent.node_ops.mknod(parent, name, mode, dev);
    },
    // helpers to create specific types of nodes
    create: async function(path, mode) {
      mode = mode !== undefined ? mode : 438 /* 0666 */;
      mode &= {{{ cDefine('S_IALLUGO') }}};
      mode |= {{{ cDefine('S_IFREG') }}};
      return await PThreadFS.mknod(path, mode, 0);
    },
    mkdir: async function(path, mode) {
      mode = mode !== undefined ? mode : 511 /* 0777 */;
      mode &= {{{ cDefine('S_IRWXUGO') }}} | {{{ cDefine('S_ISVTX') }}};
      mode |= {{{ cDefine('S_IFDIR') }}};
      return await PThreadFS.mknod(path, mode, 0);
    },
    // Creates a whole directory tree chain if it doesn't yet exist
    mkdirTree: async function(path, mode) {
      var dirs = path.split('/');
      var d = '';
      for (var i = 0; i < dirs.length; ++i) {
        if (!dirs[i]) continue;
        d += '/' + dirs[i];
        try {
          await PThreadFS.mkdir(d, mode);
        } catch(e) {
          if (e.errno != {{{ cDefine('EEXIST') }}}) throw e;
        }
      }
    },
    mkdev: async function(path, mode, dev) {
      if (typeof(dev) === 'undefined') {
        dev = mode;
        mode = 438 /* 0666 */;
      }
      mode |= {{{ cDefine('S_IFCHR') }}};
      return await PThreadFS.mknod(path, mode, dev);
    },
    symlink: async function(oldpath, newpath) {
      if (!PATH_FS.resolve(oldpath)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      var lookup = await PThreadFS.lookupPath(newpath, { parent: true });
      var parent = lookup.node;
      if (!parent) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      var newname = PATH.basename(newpath);
      var errCode = await PThreadFS.mayCreate(parent, newname);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      if (!parent.node_ops.symlink) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      return parent.node_ops.symlink(parent, newname, oldpath);
    },
    rename: async function(old_path, new_path) {
      var old_dirname = PATH.dirname(old_path);
      var new_dirname = PATH.dirname(new_path);
      var old_name = PATH.basename(old_path);
      var new_name = PATH.basename(new_path);
      // parents must exist
      var lookup, old_dir, new_dir;

      // let the errors from non existant directories percolate up
      lookup = await PThreadFS.lookupPath(old_path, { parent: true });
      old_dir = lookup.node;
      lookup = await PThreadFS.lookupPath(new_path, { parent: true });
      new_dir = lookup.node;

      if (!old_dir || !new_dir) throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      // need to be part of the same mount
      if (old_dir.mount !== new_dir.mount) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EXDEV') }}});
      }
      // source must exist
      var old_node = await PThreadFS.lookupNode(old_dir, old_name);
      // old path should not be an ancestor of the new path
      var relative = PATH_FS.relative(old_path, new_dirname);
      if (relative.charAt(0) !== '.') {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      // new path should not be an ancestor of the old path
      relative = PATH_FS.relative(new_path, old_dirname);
      if (relative.charAt(0) !== '.') {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOTEMPTY') }}});
      }
      // see if the new path already exists
      var new_node;
      try {
        new_node = await PThreadFS.lookupNode(new_dir, new_name);
      } catch (e) {
        // not fatal
      }
      // early out if nothing needs to change
      if (old_node === new_node) {
        return;
      }
      // we'll need to delete the old entry
      var isdir = PThreadFS.isDir(old_node.mode);
      var errCode = await PThreadFS.mayDelete(old_dir, old_name, isdir);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      // need delete permissions if we'll be overwriting.
      // need create permissions if new doesn't already exist.
      errCode = new_node ?
        await PThreadFS.mayDelete(new_dir, new_name, isdir) :
        await PThreadFS.mayCreate(new_dir, new_name);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      if (!old_dir.node_ops.rename) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      if (PThreadFS.isMountpoint(old_node) || (new_node && PThreadFS.isMountpoint(new_node))) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBUSY') }}});
      }
      // if we are going to change the parent, check write permissions
      if (new_dir !== old_dir) {
        errCode = PThreadFS.nodePermissions(old_dir, 'w');
        if (errCode) {
          throw new PThreadFS.ErrnoError(errCode);
        }
      }
      try {
        if (PThreadFS.trackingDelegate['willMovePath']) {
          PThreadFS.trackingDelegate['willMovePath'](old_path, new_path);
        }
      } catch(e) {
        err("PThreadFS.trackingDelegate['willMovePath']('"+old_path+"', '"+new_path+"') threw an exception: " + e.message);
      }
      // remove the node from the lookup hash
      PThreadFS.hashRemoveNode(old_node);
      // do the underlying fs rename
      try {
        await old_dir.node_ops.rename(old_node, new_dir, new_name);
      } catch (e) {
        throw e;
      } finally {
        // add the node back to the hash (in case node_ops.rename
        // changed its name)
        PThreadFS.hashAddNode(old_node);
      }
      try {
        if (PThreadFS.trackingDelegate['onMovePath']) PThreadFS.trackingDelegate['onMovePath'](old_path, new_path);
      } catch(e) {
        err("PThreadFS.trackingDelegate['onMovePath']('"+old_path+"', '"+new_path+"') threw an exception: " + e.message);
      }
    },
    rmdir: async function(path) {
      var lookup = await PThreadFS.lookupPath(path, { parent: true });
      var parent = lookup.node;
      var name = PATH.basename(path);
      var node = await PThreadFS.lookupNode(parent, name);
      var errCode = await PThreadFS.mayDelete(parent, name, true);
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      if (!parent.node_ops.rmdir) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      if (PThreadFS.isMountpoint(node)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBUSY') }}});
      }
      try {
        if (PThreadFS.trackingDelegate['willDeletePath']) {
          PThreadFS.trackingDelegate['willDeletePath'](path);
        }
      } catch(e) {
        err("PThreadFS.trackingDelegate['willDeletePath']('"+path+"') threw an exception: " + e.message);
      }
      await parent.node_ops.rmdir(parent, name);
      PThreadFS.destroyNode(node);
      try {
        if (PThreadFS.trackingDelegate['onDeletePath']) PThreadFS.trackingDelegate['onDeletePath'](path);
      } catch(e) {
        err("PThreadFS.trackingDelegate['onDeletePath']('"+path+"') threw an exception: " + e.message);
      }
    },
    readdir: async function(path) {
      var lookup = await PThreadFS.lookupPath(path, { follow: true });
      var node = lookup.node;
      if (!node.node_ops.readdir) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOTDIR') }}});
      }
      return await node.node_ops.readdir(node);
    },
    unlink: async function(path) {
      var lookup = await PThreadFS.lookupPath(path, { parent: true });
      var parent = lookup.node;
      var name = PATH.basename(path);
      var node = await PThreadFS.lookupNode(parent, name);
      var errCode = await PThreadFS.mayDelete(parent, name, false);
      if (errCode) {
        // According to POSIX, we should map EISDIR to EPERM, but
        // we instead do what Linux does (and we must, as we use
        // the musl linux libc).
        throw new PThreadFS.ErrnoError(errCode);
      }
      if (!parent.node_ops.unlink) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      if (PThreadFS.isMountpoint(node)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBUSY') }}});
      }
      try {
        if (PThreadFS.trackingDelegate['willDeletePath']) {
          PThreadFS.trackingDelegate['willDeletePath'](path);
        }
      } catch(e) {
        err("PThreadFS.trackingDelegate['willDeletePath']('"+path+"') threw an exception: " + e.message);
      }
      await parent.node_ops.unlink(parent, name);
      PThreadFS.destroyNode(node);
      try {
        if (PThreadFS.trackingDelegate['onDeletePath']) PThreadFS.trackingDelegate['onDeletePath'](path);
      } catch(e) {
        err("PThreadFS.trackingDelegate['onDeletePath']('"+path+"') threw an exception: " + e.message);
      }
    },
    readlink: async function(path) {
      var lookup = await PThreadFS.lookupPath(path);
      var link = lookup.node;
      if (!link) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      if (!link.node_ops.readlink) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      return PATH_FS.resolve(PThreadFS.getPath(link.parent), link.node_ops.readlink(link));
    },
    stat: async function(path, dontFollow) {
      var lookup = await PThreadFS.lookupPath(path, { follow: !dontFollow });
      var node = lookup.node;
      if (!node) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      if (!node.node_ops.getattr) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      return await node.node_ops.getattr(node);
    },
    lstat: async function(path) {
      return await PThreadFS.stat(path, true);
    },
    chmod: async function(path, mode, dontFollow) {
      var node;
      if (typeof path === 'string') {
        var lookup = await PThreadFS.lookupPath(path, { follow: !dontFollow });
        node = lookup.node;
      } else {
        node = path;
      }
      if (!node.node_ops.setattr) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      await node.node_ops.setattr(node, {
        mode: (mode & {{{ cDefine('S_IALLUGO') }}}) | (node.mode & ~{{{ cDefine('S_IALLUGO') }}}),
        timestamp: Date.now()
      });
    },
    lchmod: async function(path, mode) {
      await PThreadFS.chmod(path, mode, true);
    },
    fchmod: async function(fd, mode) {
      var stream = PThreadFS.getStream(fd);
      if (!stream) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      await PThreadFS.chmod(stream.node, mode);
    },
    chown: async function(path, uid, gid, dontFollow) {
      var node;
      if (typeof path === 'string') {
        var lookup = await PThreadFS.lookupPath(path, { follow: !dontFollow });
        node = lookup.node;
      } else {
        node = path;
      }
      if (!node.node_ops.setattr) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      await node.node_ops.setattr(node, {
        timestamp: Date.now()
        // we ignore the uid / gid for now
      });
    },
    lchown: async function(path, uid, gid) {
      await PThreadFS.chown(path, uid, gid, true);
    },
    fchown: async function(fd, uid, gid) {
      var stream = PThreadFS.getStream(fd);
      if (!stream) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      await PThreadFS.chown(stream.node, uid, gid);
    },
    truncate: async function(path, len) {
      if (len < 0) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var node;
      if (typeof path === 'string') {
        var lookup = await PThreadFS.lookupPath(path, { follow: true });
        node = lookup.node;
      } else {
        node = path;
      }
      if (!node.node_ops.setattr) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EPERM') }}});
      }
      if (PThreadFS.isDir(node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EISDIR') }}});
      }
      if (!PThreadFS.isFile(node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var errCode = PThreadFS.nodePermissions(node, 'w');
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      await node.node_ops.setattr(node, {
        size: len,
        timestamp: Date.now()
      });
   },
    ftruncate: async function(fd, len) {
      var stream = PThreadFS.getStream(fd);
      if (!stream) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_RDONLY')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      await PThreadFS.truncate(stream.node, len);
    },
    utime: async function(path, atime, mtime) {
      var lookup = await PThreadFS.lookupPath(path, { follow: true });
      var node = lookup.node;
      await node.node_ops.setattr(node, {
        timestamp: Math.max(atime, mtime)
      });
   },
    open: async function(path, flags, mode, fd_start, fd_end) {
      if (path === "") {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      flags = typeof flags === 'string' ? PThreadFS.modeStringToFlags(flags) : flags;
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
          var lookup = await PThreadFS.lookupPath(path, {
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
            throw new PThreadFS.ErrnoError({{{ cDefine('EEXIST') }}});
          }
        } else {
          // node doesn't exist, try to create it
          node = await PThreadFS.mknod(path, mode, 0);
          created = true;
        }
      }
      if (!node) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      // can't truncate a device
      if (PThreadFS.isChrdev(node.mode)) {
        flags &= ~{{{ cDefine('O_TRUNC') }}};
      }
      // if asked only for a directory, then this must be one
      if ((flags & {{{ cDefine('O_DIRECTORY') }}}) && !PThreadFS.isDir(node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOTDIR') }}});
      }
      // check permissions, if this is not a file we just created now (it is ok to
      // create and write to a file with read-only permissions; it is read-only
      // for later use)
      if (!created) {
        var errCode = PThreadFS.mayOpen(node, flags);
        if (errCode) {
          throw new PThreadFS.ErrnoError(errCode);
        }
      }
      // do truncation if necessary
      if ((flags & {{{ cDefine('O_TRUNC')}}})) {
        await PThreadFS.truncate(node, 0);
      }
      // we've already handled these, don't pass down to the underlying vfs
      flags &= ~({{{ cDefine('O_EXCL') }}} | {{{ cDefine('O_TRUNC') }}} | {{{ cDefine('O_NOFOLLOW') }}});

      // register the stream with the filesystem
      var stream = PThreadFS.createStream({
        node: node,
        path: PThreadFS.getPath(node),  // we want the absolute path to the node
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
        await stream.stream_ops.open(stream);
      }
      if (Module['logReadFiles'] && !(flags & {{{ cDefine('O_WRONLY')}}})) {
        if (!PThreadFS.readFiles) PThreadFS.readFiles = {};
        if (!(path in PThreadFS.readFiles)) {
          PThreadFS.readFiles[path] = 1;
          err("PThreadFS.trackingDelegate error on read file: " + path);
        }
      }
      try {
        if (PThreadFS.trackingDelegate['onOpenFile']) {
          var trackingFlags = 0;
          if ((flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_WRONLY') }}}) {
            trackingFlags |= PThreadFS.tracking.openFlags.READ;
          }
          if ((flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_RDONLY') }}}) {
            trackingFlags |= PThreadFS.tracking.openFlags.WRITE;
          }
          PThreadFS.trackingDelegate['onOpenFile'](path, trackingFlags);
        }
      } catch(e) {
        err("PThreadFS.trackingDelegate['onOpenFile']('"+path+"', flags) threw an exception: " + e.message);
      }
      return stream;
    },
    close: async function(stream) {
      if (PThreadFS.isClosed(stream)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (stream.getdents) stream.getdents = null; // free readdir state
      try {
        if (stream.stream_ops.close) {
          await stream.stream_ops.close(stream);
        }
      } catch (e) {
        throw e;
      } finally {
        PThreadFS.closeStream(stream.fd);
      }
      stream.fd = null;
    },
    isClosed: function(stream) {
      return stream.fd === null;
    },
    llseek: async function(stream, offset, whence) {
      if (PThreadFS.isClosed(stream)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (!stream.seekable || !stream.stream_ops.llseek) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ESPIPE') }}});
      }
      if (whence != {{{ cDefine('SEEK_SET') }}} && whence != {{{ cDefine('SEEK_CUR') }}} && whence != {{{ cDefine('SEEK_END') }}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      stream.position = await stream.stream_ops.llseek(stream, offset, whence);
      stream.ungotten = [];
      return stream.position;
    },
    read: async function(stream, buffer, offset, length, position) {
#if CAN_ADDRESS_2GB
      offset >>>= 0;
#endif
      if (length < 0 || position < 0) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      if (PThreadFS.isClosed(stream)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_WRONLY')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (PThreadFS.isDir(stream.node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EISDIR') }}});
      }
      if (!stream.stream_ops.read) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      var seeking = typeof position !== 'undefined';
      if (!seeking) {
        position = stream.position;
      } else if (!stream.seekable) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ESPIPE') }}});
      }
      var bytesRead = await stream.stream_ops.read(stream, buffer, offset, length, position);
      if (!seeking) stream.position += bytesRead;
      return bytesRead;
    },
    write: async function(stream, buffer, offset, length, position, canOwn) {
#if CAN_ADDRESS_2GB
      offset >>>= 0;
#endif
      if (length < 0 || position < 0) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      if (PThreadFS.isClosed(stream)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_RDONLY')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (PThreadFS.isDir(stream.node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EISDIR') }}});
      }
      if (!stream.stream_ops.write) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      if (stream.seekable && stream.flags & {{{ cDefine('O_APPEND') }}}) {
        // seek to the end before writing in append mode
        await PThreadFS.llseek(stream, 0, {{{ cDefine('SEEK_END') }}});
      }
      var seeking = typeof position !== 'undefined';
      if (!seeking) {
        position = stream.position;
      } else if (!stream.seekable) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ESPIPE') }}});
      }
      var bytesWritten = await stream.stream_ops.write(stream, buffer, offset, length, position, canOwn);
      if (!seeking) stream.position += bytesWritten;
      try {
        if (stream.path && PThreadFS.trackingDelegate['onWriteToFile']) PThreadFS.trackingDelegate['onWriteToFile'](stream.path);
      } catch(e) {
        err("PThreadFS.trackingDelegate['onWriteToFile']('"+stream.path+"') threw an exception: " + e.message);
      }
      return bytesWritten;
    },
    allocate: async function(stream, offset, length) {
      if (PThreadFS.isClosed(stream)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (offset < 0 || length <= 0) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EINVAL') }}});
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_RDONLY')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
      }
      if (!PThreadFS.isFile(stream.node.mode) && !PThreadFS.isDir(stream.node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENODEV') }}});
      }
      if (!stream.stream_ops.allocate) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EOPNOTSUPP') }}});
      }
      await stream.stream_ops.allocate(stream, offset, length);
   },
    mmap: async function(stream, address, length, position, prot, flags) {
#if CAN_ADDRESS_2GB
      address >>>= 0;
#endif
      // User requests writing to file (prot & PROT_WRITE != 0).
      // Checking if we have permissions to write to the file unless
      // MAP_PRIVATE flag is set. According to POSIX spec it is possible
      // to write to file opened in read-only mode with MAP_PRIVATE flag,
      // as all modifications will be visible only in the memory of
      // the current process.
      if ((prot & {{{ cDefine('PROT_WRITE') }}}) !== 0
          && (flags & {{{ cDefine('MAP_PRIVATE')}}}) === 0
          && (stream.flags & {{{ cDefine('O_ACCMODE') }}}) !== {{{ cDefine('O_RDWR')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EACCES') }}});
      }
      if ((stream.flags & {{{ cDefine('O_ACCMODE') }}}) === {{{ cDefine('O_WRONLY')}}}) {
        throw new PThreadFS.ErrnoError({{{ cDefine('EACCES') }}});
      }
      if (!stream.stream_ops.mmap) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENODEV') }}});
      }
      return await stream.stream_ops.mmap(stream, address, length, position, prot, flags);
    },
    msync: async function(stream, buffer, offset, length, mmapFlags) {
#if CAN_ADDRESS_2GB
      offset >>>= 0;
#endif
      if (!stream || !stream.stream_ops.msync) {
        return 0;
      }
      return await stream.stream_ops.msync(stream, buffer, offset, length, mmapFlags);
    },
    munmap: function(stream) {
      return 0;
    },
    ioctl: async function(stream, cmd, arg) {
      if (!stream.stream_ops.ioctl) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOTTY') }}});
      }
      return await stream.stream_ops.ioctl(stream, cmd, arg);
    },
    readFile: async function(path, opts) {
      opts = opts || {};
      opts.flags = opts.flags || {{{ cDefine('O_RDONLY') }}};
      opts.encoding = opts.encoding || 'binary';
      if (opts.encoding !== 'utf8' && opts.encoding !== 'binary') {
        throw new Error('Invalid encoding type "' + opts.encoding + '"');
      }
      var ret;
      var stream = await PThreadFS.open(path, opts.flags);
      var stat = await PThreadFS.stat(path);
      var length = stat.size;
      var buf = new Uint8Array(length);
      await PThreadFS.read(stream, buf, 0, length, 0);
      if (opts.encoding === 'utf8') {
        ret = UTF8ArrayToString(buf, 0);
      } else if (opts.encoding === 'binary') {
        ret = buf;
      }
      await PThreadFS.close(stream);
      return ret;
    },
    writeFile: async function(path, data, opts) {
      opts = opts || {};
      opts.flags = opts.flags || {{{ cDefine('O_TRUNC') | cDefine('O_CREAT') | cDefine('O_WRONLY') }}};
      var stream = await PThreadFS.open(path, opts.flags, opts.mode);
      if (typeof data === 'string') {
        var buf = new Uint8Array(lengthBytesUTF8(data)+1);
        var actualNumBytes = stringToUTF8Array(data, buf, 0, buf.length);
        await PThreadFS.write(stream, buf, 0, actualNumBytes, undefined, opts.canOwn);
      } else if (ArrayBuffer.isView(data)) {
        await PThreadFS.write(stream, data, 0, data.byteLength, undefined, opts.canOwn);
      } else {
        throw new Error('Unsupported data type');
      }
      await PThreadFS.close(stream);
    },

    //
    // module-level FS code
    //
    cwd: function() {
      return PThreadFS.currentPath;
    },
    chdir: async function(path) {
      var lookup = await PThreadFS.lookupPath(path, { follow: true });
      if (lookup.node === null) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOENT') }}});
      }
      if (!PThreadFS.isDir(lookup.node.mode)) {
        throw new PThreadFS.ErrnoError({{{ cDefine('ENOTDIR') }}});
      }
      var errCode = PThreadFS.nodePermissions(lookup.node, 'x');
      if (errCode) {
        throw new PThreadFS.ErrnoError(errCode);
      }
      PThreadFS.currentPath = lookup.path;
    },
    createDefaultDirectories: async function() {
      await PThreadFS.mkdir('/tmp');
      await PThreadFS.mkdir('/home');
      await PThreadFS.mkdir('/home/web_user');
    },
    createDefaultDevices: async function() {
      // create /dev
      await PThreadFS.mkdir('/dev');
      // setup /dev/null
      PThreadFS.registerDevice(PThreadFS.makedev(1, 3), {
        read: function() { return 0; },
        write: function(stream, buffer, offset, length, pos) { return length; }
      });
      await PThreadFS.mkdev('/dev/null', PThreadFS.makedev(1, 3));
      // setup /dev/tty and /dev/tty1
      // stderr needs to print output using err() rather than out()
      // so we register a second tty just for it.
      TTY_ASYNC.register(PThreadFS.makedev(5, 0), TTY_ASYNC.default_tty_ops);
      TTY_ASYNC.register(PThreadFS.makedev(6, 0), TTY_ASYNC.default_tty1_ops);
      await PThreadFS.mkdev('/dev/tty', PThreadFS.makedev(5, 0));
      await PThreadFS.mkdev('/dev/tty1', PThreadFS.makedev(6, 0));
      // setup /dev/[u]random
      var random_device = getRandomDevice();
      await PThreadFS.createDevice('/dev', 'random', random_device);
      await PThreadFS.createDevice('/dev', 'urandom', random_device);
      // we're not going to emulate the actual shm device,
      // just create the tmp dirs that reside in it commonly
      await PThreadFS.mkdir('/dev/shm');
      await PThreadFS.mkdir('/dev/shm/tmp');
    },
    createSpecialDirectories: async function() {
      // create /proc/self/fd which allows /proc/self/fd/6 => readlink gives the
      // name of the stream for fd 6 (see test_unistd_ttyname)
      await PThreadFS.mkdir('/proc');
      var proc_self = await PThreadFS.mkdir('/proc/self');
      await PThreadFS.mkdir('/proc/self/fd');
      await PThreadFS.mount({
        mount: function() {
          var node = PThreadFS.createNode(proc_self, 'fd', {{{ cDefine('S_IFDIR') }}} | 511 /* 0777 */, {{{ cDefine('S_IXUGO') }}});
          node.node_ops = {
            lookup: function(parent, name) {
              var fd = +name;
              var stream = PThreadFS.getStream(fd);
              if (!stream) throw new PThreadFS.ErrnoError({{{ cDefine('EBADF') }}});
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
    createStandardStreams: async function() {
      // TODO deprecate the old functionality of a single
      // input / output callback and that utilizes PThreadFS.createDevice
      // and instead require a unique set of stream ops

      // by default, we symlink the standard streams to the
      // default tty devices. however, if the standard streams
      // have been overwritten we create a unique device for
      // them instead.
      if (Module['stdin']) {
        await PThreadFS.createDevice('/dev', 'stdin', Module['stdin']);
      } else {
        await PThreadFS.symlink('/dev/tty', '/dev/stdin');
      }
      if (Module['stdout']) {
        await PThreadFS.createDevice('/dev', 'stdout', null, Module['stdout']);
      } else {
        await PThreadFS.symlink('/dev/tty', '/dev/stdout');
      }
      if (Module['stderr']) {
        await PThreadFS.createDevice('/dev', 'stderr', null, Module['stderr']);
      } else {
        await PThreadFS.symlink('/dev/tty1', '/dev/stderr');
      }

      // open default streams for the stdin, stdout and stderr devices
      var stdin = await PThreadFS.open('/dev/stdin', {{{ cDefine('O_RDONLY') }}});
      var stdout = await PThreadFS.open('/dev/stdout', {{{ cDefine('O_WRONLY') }}});
      var stderr = await PThreadFS.open('/dev/stderr', {{{ cDefine('O_WRONLY') }}});
#if ASSERTIONS
      assert(stdin.fd === 0, 'invalid handle for stdin (' + stdin.fd + ')');
      assert(stdout.fd === 1, 'invalid handle for stdout (' + stdout.fd + ')');
      assert(stderr.fd === 2, 'invalid handle for stderr (' + stderr.fd + ')');
#endif
    },
    ensureErrnoError: function() {
      if (PThreadFS.ErrnoError) return;
      PThreadFS.ErrnoError = /** @this{Object} */ function ErrnoError(errno, node) {
        this.node = node;
        this.setErrno = /** @this{Object} */ function(errno) {
          this.errno = errno;
#if ASSERTIONS
          for (var key in ERRNO_CODES) {
            if (ERRNO_CODES[key] === errno) {
              this.code = key;
              break;
            }
          }
#endif
        };
        this.setErrno(errno);
#if ASSERTIONS
        this.message = ERRNO_MESSAGES[errno];
#else
        this.message = 'PThreadFS error';
#endif

#if ASSERTIONS && !MINIMAL_RUNTIME
        // Try to get a maximally helpful stack trace. On Node.js, getting Error.stack
        // now ensures it shows what we want.
        if (this.stack) {
          // Define the stack property for Node.js 4, which otherwise errors on the next line.
          Object.defineProperty(this, "stack", { value: (new Error).stack, writable: true });
          this.stack = demangleAll(this.stack);
        }
#endif // ASSERTIONS
      };
      PThreadFS.ErrnoError.prototype = new Error();
      PThreadFS.ErrnoError.prototype.constructor = PThreadFS.ErrnoError;
      // Some errors may happen quite a bit, to avoid overhead we reuse them (and suffer a lack of stack info)
      [{{{ cDefine('ENOENT') }}}].forEach(function(code) {
        PThreadFS.genericErrors[code] = new PThreadFS.ErrnoError(code);
        PThreadFS.genericErrors[code].stack = '<generic error, no stack>';
      });
    },
    staticInit: async function() {
      PThreadFS.ensureErrnoError();

      PThreadFS.nameTable = new Array(4096);

      await PThreadFS.mount(MEMFS_ASYNC, {}, '/');

      await PThreadFS.createDefaultDirectories();
      await PThreadFS.createDefaultDevices();
      await PThreadFS.createSpecialDirectories();

      PThreadFS.filesystems = {
        'MEMFS_ASYNC': MEMFS_ASYNC,
      };
    },
    init: async function(input, output, error) {
#if ASSERTIONS
      assert(!PThreadFS.init.initialized, 'PThreadFS.init was previously called. If you want to initialize later with custom parameters, remove any earlier calls (note that one is automatically added to the generated code)');
#endif
      PThreadFS.init.initialized = true;

      PThreadFS.ensureErrnoError();

      // Allow Module.stdin etc. to provide defaults, if none explicitly passed to us here
      Module['stdin'] = input || Module['stdin'];
      Module['stdout'] = output || Module['stdout'];
      Module['stderr'] = error || Module['stderr'];

      await PThreadFS.createStandardStreams();
    },
    quit: async function() {
      // TODO(rstz): This function is never called.
      PThreadFS.init.initialized = false;
      // force-flush all streams, so we get musl std streams printed out
      var fflush = Module['_fflush'];
      if (fflush) fflush(0);
      // close all of our streams
      for (var i = 0; i < PThreadFS.streams.length; i++) {
        var stream = PThreadFS.streams[i];
        if (!stream) {
          continue;
        }
        await PThreadFS.close(stream);
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
    findObject: async function(path, dontResolveLastLink) {
      var ret = await PThreadFS.analyzePath(path, dontResolveLastLink);
      if (ret.exists) {
        return ret.object;
      } else {
        return null;
      }
    },
    analyzePath: async function(path, dontResolveLastLink) {
      // operate from within the context of the symlink's target
      try {
        var lookup = await PThreadFS.lookupPath(path, { follow: !dontResolveLastLink });
        path = lookup.path;
      } catch (e) {
      }
      var ret = {
        isRoot: false, exists: false, error: 0, name: null, path: null, object: null,
        parentExists: false, parentPath: null, parentObject: null
      };
      try {
        var lookup = await PThreadFS.lookupPath(path, { parent: true });
        ret.parentExists = true;
        ret.parentPath = lookup.path;
        ret.parentObject = lookup.node;
        ret.name = PATH.basename(path);
        lookup = await PThreadFS.lookupPath(path, { follow: !dontResolveLastLink });
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
    createPath: async function(parent, path, canRead, canWrite) {
      parent = typeof parent === 'string' ? parent : PThreadFS.getPath(parent);
      var parts = path.split('/').reverse();
      while (parts.length) {
        var part = parts.pop();
        if (!part) continue;
        var current = PATH.join2(parent, part);
        try {
          await PThreadFS.mkdir(current);
        } catch (e) {
          // ignore EEXIST
        }
        parent = current;
      }
      return current;
    },
    createFile: async function(parent, name, properties, canRead, canWrite) {
      var path = PATH.join2(typeof parent === 'string' ? parent : await PThreadFS.getPath(parent), name);
      var mode = await PThreadFS.getMode(canRead, canWrite);
      return await PThreadFS.create(path, mode);
    },
    createDataFile: async function(parent, name, data, canRead, canWrite, canOwn) {
      var path = name ? PATH.join2(typeof parent === 'string' ? parent : await PThreadFS.getPath(parent), name) : parent;
      var mode = await PThreadFS.getMode(canRead, canWrite);
      var node = await PThreadFS.create(path, mode);
      if (data) {
        if (typeof data === 'string') {
          var arr = new Array(data.length);
          for (var i = 0, len = data.length; i < len; ++i) arr[i] = data.charCodeAt(i);
          data = arr;
        }
        // make sure we can write to the file
        await PThreadFS.chmod(node, mode | {{{ cDefine('S_IWUGO') }}});
        var stream = await PThreadFS.open(node, {{{ cDefine('O_TRUNC') | cDefine('O_CREAT') | cDefine('O_WRONLY') }}});
        await PThreadFS.write(stream, data, 0, data.length, 0, canOwn);
        await PThreadFS.close(stream);
        await PThreadFS.chmod(node, mode);
      }
      return node;
    },
    createDevice: async function(parent, name, input, output) {
      var path = PATH.join2(typeof parent === 'string' ? parent : PThreadFS.getPath(parent), name);
      var mode = PThreadFS.getMode(!!input, !!output);
      if (!PThreadFS.createDevice.major) PThreadFS.createDevice.major = 64;
      var dev = PThreadFS.makedev(PThreadFS.createDevice.major++, 0);
      // Create a fake device that a set of stream ops to emulate
      // the old behavior.
      PThreadFS.registerDevice(dev, {
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
              throw new PThreadFS.ErrnoError({{{ cDefine('EIO') }}});
            }
            if (result === undefined && bytesRead === 0) {
              throw new PThreadFS.ErrnoError({{{ cDefine('EAGAIN') }}});
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
              throw new PThreadFS.ErrnoError({{{ cDefine('EIO') }}});
            }
          }
          if (length) {
            stream.node.timestamp = Date.now();
          }
          return i;
        }
      });
      return await PThreadFS.mkdev(path, mode, dev);
    },
    // Makes sure a file's contents are loaded. Returns whether the file has
    // been loaded successfully. No-op for files that have been loaded already.
    forceLoadFile: function(obj) {
      if (obj.isDevice || obj.isFolder || obj.link || obj.contents) return true;
      if (typeof XMLHttpRequest !== 'undefined') {
        throw new Error("Lazy loading should have been performed (contents set) in createLazyFile, but it was not. Lazy loading only works in web workers. Use --embed-file or --preload-file in emcc on the main thread.");
      } else if (read_) {
        // Command-line.
        try {
          // WARNING: Can't read binary files in V8's d8 or tracemonkey's js, as
          //          read() will try to parse UTF8.
          obj.contents = intArrayFromString(read_(obj.url), true);
          obj.usedBytes = obj.contents.length;
        } catch (e) {
          throw new PThreadFS.ErrnoError({{{ cDefine('EIO') }}});
        }
      } else {
        throw new Error('Cannot load without read() or XMLHttpRequest.');
      }
    },
  },
});