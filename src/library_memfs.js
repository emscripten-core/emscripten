mergeInto(LibraryManager.library, {
  $MEMFS__deps: ['$FS'],
  $MEMFS: {
    // content modes
    CONTENT_OWNING: 1, // contains a subarray into the heap, and we own it, without copying (note: someone else needs to free() it, if that is necessary)
    CONTENT_FLEXIBLE: 2, // has been modified or never set to anything, and is a flexible js array that can grow/shrink
    CONTENT_FIXED: 3, // contains some fixed-size content written into it, in a typed array
    mount: function(mount) {
      return MEMFS.createNode(null, '/', {{{ cDefine('S_IFDIR') }}} | 0777, 0);
    },
    createNode: function(parent, name, mode, dev) {
      if (FS.isBlkdev(mode) || FS.isFIFO(mode)) {
        // no supported
        throw new FS.ErrnoError(ERRNO_CODES.EPERM);
      }
      var node = FS.createNode(parent, name, mode, dev);
      if (FS.isDir(node.mode)) {
        node.node_ops = {
          getattr: MEMFS.node_ops.getattr,
          setattr: MEMFS.node_ops.setattr,
          lookup: MEMFS.node_ops.lookup,
          mknod: MEMFS.node_ops.mknod,
          mknod: MEMFS.node_ops.mknod,
          rename: MEMFS.node_ops.rename,
          unlink: MEMFS.node_ops.unlink,
          rmdir: MEMFS.node_ops.rmdir,
          readdir: MEMFS.node_ops.readdir,
          symlink: MEMFS.node_ops.symlink
        };
        node.stream_ops = {
          llseek: MEMFS.stream_ops.llseek
        };
        node.contents = {};
      } else if (FS.isFile(node.mode)) {
        node.node_ops = {
          getattr: MEMFS.node_ops.getattr,
          setattr: MEMFS.node_ops.setattr
        };
        node.stream_ops = {
          llseek: MEMFS.stream_ops.llseek,
          read: MEMFS.stream_ops.read,
          write: MEMFS.stream_ops.write,
          allocate: MEMFS.stream_ops.allocate,
          mmap: MEMFS.stream_ops.mmap
        };
        node.contents = [];
        node.contentMode = MEMFS.CONTENT_FLEXIBLE;
      } else if (FS.isLink(node.mode)) {
        node.node_ops = {
          getattr: MEMFS.node_ops.getattr,
          setattr: MEMFS.node_ops.setattr,
          readlink: MEMFS.node_ops.readlink
        };
        node.stream_ops = {};
      } else if (FS.isChrdev(node.mode)) {
        node.node_ops = {
          getattr: MEMFS.node_ops.getattr,
          setattr: MEMFS.node_ops.setattr
        };
        node.stream_ops = FS.chrdev_stream_ops;
      }
      node.timestamp = Date.now();
      // add the new node to the parent
      if (parent) {
        parent.contents[name] = node;
      }
      return node;
    },
    ensureFlexible: function(node) {
      if (node.contentMode !== MEMFS.CONTENT_FLEXIBLE) {
        var contents = node.contents;
        node.contents = Array.prototype.slice.call(contents);
        node.contentMode = MEMFS.CONTENT_FLEXIBLE;
      }
    },
    node_ops: {
      getattr: function(node) {
        var attr = {};
        // device numbers reuse inode numbers.
        attr.dev = FS.isChrdev(node.mode) ? node.id : 1;
        attr.ino = node.id;
        attr.mode = node.mode;
        attr.nlink = 1;
        attr.uid = 0;
        attr.gid = 0;
        attr.rdev = node.rdev;
        if (FS.isDir(node.mode)) {
          attr.size = 4096;
        } else if (FS.isFile(node.mode)) {
          attr.size = node.contents.length;
        } else if (FS.isLink(node.mode)) {
          attr.size = node.link.length;
        } else {
          attr.size = 0;
        }
        attr.atime = new Date(node.timestamp);
        attr.mtime = new Date(node.timestamp);
        attr.ctime = new Date(node.timestamp);
        // NOTE: In our implementation, st_blocks = Math.ceil(st_size/st_blksize),
        //       but this is not required by the standard.
        attr.blksize = 4096;
        attr.blocks = Math.ceil(attr.size / attr.blksize);
        return attr;
      },
      setattr: function(node, attr) {
        if (attr.mode !== undefined) {
          node.mode = attr.mode;
        }
        if (attr.timestamp !== undefined) {
          node.timestamp = attr.timestamp;
        }
        if (attr.size !== undefined) {
          MEMFS.ensureFlexible(node);
          var contents = node.contents;
          if (attr.size < contents.length) contents.length = attr.size;
          else while (attr.size > contents.length) contents.push(0);
        }
      },
      lookup: function(parent, name) {
        throw new FS.ErrnoError(ERRNO_CODES.ENOENT);
      },
      mknod: function(parent, name, mode, dev) {
        return MEMFS.createNode(parent, name, mode, dev);
      },
      rename: function(old_node, new_dir, new_name) {
        // if we're overwriting a directory at new_name, make sure it's empty.
        if (FS.isDir(old_node.mode)) {
          var new_node;
          try {
            new_node = FS.lookupNode(new_dir, new_name);
          } catch (e) {
          }
          if (new_node) {
            for (var i in new_node.contents) {
              throw new FS.ErrnoError(ERRNO_CODES.ENOTEMPTY);
            }
          }
        }
        // do the internal rewiring
        delete old_node.parent.contents[old_node.name];
        old_node.name = new_name;
        new_dir.contents[new_name] = old_node;
        old_node.parent = new_dir;
      },
      unlink: function(parent, name) {
        delete parent.contents[name];
      },
      rmdir: function(parent, name) {
        var node = FS.lookupNode(parent, name);
        for (var i in node.contents) {
          throw new FS.ErrnoError(ERRNO_CODES.ENOTEMPTY);
        }
        delete parent.contents[name];
      },
      readdir: function(node) {
        var entries = ['.', '..']
        for (var key in node.contents) {
          if (!node.contents.hasOwnProperty(key)) {
            continue;
          }
          entries.push(key);
        }
        return entries;
      },
      symlink: function(parent, newname, oldpath) {
        var node = MEMFS.createNode(parent, newname, 0777 | {{{ cDefine('S_IFLNK') }}}, 0);
        node.link = oldpath;
        return node;
      },
      readlink: function(node) {
        if (!FS.isLink(node.mode)) {
          throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
        }
        return node.link;
      },
    },
    stream_ops: {
      read: function(stream, buffer, offset, length, position) {
        var contents = stream.node.contents;
        if (position >= contents.length)
          return 0;
        var size = Math.min(contents.length - position, length);
        assert(size >= 0);
#if USE_TYPED_ARRAYS == 2
        if (size > 8 && contents.subarray) { // non-trivial, and typed array
          buffer.set(contents.subarray(position, position + size), offset);
        } else
#endif
        {
          for (var i = 0; i < size; i++) {
            buffer[offset + i] = contents[position + i];
          }
        }
        return size;
      },
      write: function(stream, buffer, offset, length, position, canOwn) {
        var node = stream.node;
        node.timestamp = Date.now();
        var contents = node.contents;
#if USE_TYPED_ARRAYS == 2
        if (length && contents.length === 0 && position === 0 && buffer.subarray) {
          // just replace it with the new data
          assert(buffer.length);
          if (canOwn && buffer.buffer === HEAP8.buffer && offset === 0) {
            node.contents = buffer; // this is a subarray of the heap, and we can own it
            node.contentMode = MEMFS.CONTENT_OWNING;
          } else {
            node.contents = new Uint8Array(buffer.subarray(offset, offset+length));
            node.contentMode = MEMFS.CONTENT_FIXED;
          }
          return length;
        }
#endif
        MEMFS.ensureFlexible(node);
        var contents = node.contents;
        while (contents.length < position) contents.push(0);
        for (var i = 0; i < length; i++) {
          contents[position + i] = buffer[offset + i];
        }
        return length;
      },
      llseek: function(stream, offset, whence) {
        var position = offset;
        if (whence === 1) {  // SEEK_CUR.
          position += stream.position;
        } else if (whence === 2) {  // SEEK_END.
          if (FS.isFile(stream.node.mode)) {
            position += stream.node.contents.length;
          }
        }
        if (position < 0) {
          throw new FS.ErrnoError(ERRNO_CODES.EINVAL);
        }
        stream.ungotten = [];
        stream.position = position;
        return position;
      },
      allocate: function(stream, offset, length) {
        MEMFS.ensureFlexible(stream.node);
        var contents = stream.node.contents;
        var limit = offset + length;
        while (limit > contents.length) contents.push(0);
      },
      mmap: function(stream, buffer, offset, length, position, prot, flags) {
        if (!FS.isFile(stream.node.mode)) {
          throw new FS.ErrnoError(ERRNO_CODES.ENODEV);
        }
        var ptr;
        var allocated;
        var contents = stream.node.contents;
        // Only make a new copy when MAP_PRIVATE is specified.
        if ( !(flags & {{{ cDefine('MAP_PRIVATE') }}}) &&
              (contents.buffer === buffer || contents.buffer === buffer.buffer) ) {
          // We can't emulate MAP_SHARED when the file is not backed by the buffer
          // we're mapping to (e.g. the HEAP buffer).
          allocated = false;
          ptr = contents.byteOffset;
        } else {
          // Try to avoid unnecessary slices.
          if (position > 0 || position + length < contents.length) {
            if (contents.subarray) {
              contents = contents.subarray(position, position + length);
            } else {
              contents = Array.prototype.slice.call(contents, position, position + length);
            }
          }
          allocated = true;
          ptr = _malloc(length);
          if (!ptr) {
            throw new FS.ErrnoError(ERRNO_CODES.ENOMEM);
          }
          buffer.set(contents, ptr);
        }
        return { ptr: ptr, allocated: allocated };
      },
    }
  }
});

